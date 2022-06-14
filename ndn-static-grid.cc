/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021–2022 Universidade de Vigo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Miguel Rodríguez Pérez <miguel@det.uvigo.gal>
 */

#include "icarus-grid-helper.hpp"
#include "icarus-grid-tracer.hpp"
#include "icarus-router-helper.hpp"

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "src/core/model/uinteger.h"
#include <cstddef>
#include <sstream>
#include <vector>

#include <boost/tokenizer.hpp>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NdnStaticGridSimulation");

namespace ns3 {
namespace icarus {

auto
vec_from_string(const std::string& str) -> std::vector<std::size_t>
{
  std::vector<std::size_t> values;

  const boost::tokenizer<> tok(str);
  std::transform(tok.begin(), tok.end(), back_inserter(values),
                 [](auto val) { return atoi(val.c_str()); });

  return values;
}

template <typename T>
auto
abs_diff(const T a, const T b) -> T
{
  return std::max(a, b) - std::min(a, b);
}

auto
main(int argc, char** argv)
{
  std::size_t rows = 10, columns = 10, clients = 1, cache_size = 10;
  std::string routerHelperName = "Stochastic"s;
  std::string outPrefix = "results/"s;
  std::string hcaches_list, vcaches_list;
  ns3::Time duration = Seconds(2.0);

  // Setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1000Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("1ms"));
  Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("100p"));

  CommandLine cmd;
  cmd.AddValue("rate", "ns3::PointToPointNetDevice::DataRate");
  cmd.AddValue("delay", "ns3::PointToPointChannel::Delay");
  cmd.AddValue("duration", "Simulation duration", duration);
  cmd.AddValue("r", "Number of rows", rows);
  cmd.AddValue("c", "Number of columns", columns);
  cmd.AddValue("clients", "Number of clients", clients);
  cmd.AddValue("cache", "Cache size", cache_size);
  cmd.AddValue("router", "Router helper algorithm", routerHelperName);
  cmd.AddValue("prefix", "Prefix for the output files", outPrefix);
  cmd.AddValue("hcaches", "Location of the horizontal caches", hcaches_list);
  cmd.AddValue("vcaches", "Location of the vertical caches", vcaches_list);

  cmd.Parse(argc, argv);

  auto uniformRandomVar = CreateObject<UniformRandomVariable>();

  const auto hcaches = vec_from_string(hcaches_list);
  const auto vcaches = vec_from_string(vcaches_list);

  PointToPointHelper p2p;
  IcarusGridHelper grid(rows, columns, p2p);

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.setPolicy("nfd::cs::lru");

  // We have to install NDN node by node so that their cache configuration
  // can be different. A proper way to fix it is to use a different caching
  // strategy that takes into account the producer location.

  const size_t producer_location_row = rows / 2;
  const size_t producer_location_column = columns / 2;

  for (std::size_t row = 0; row < rows; row++) {
    for (std::size_t column = 0; column < columns; column++) {
      if ((abs_diff(row, producer_location_row) == 0
           && find(hcaches.begin(), hcaches.end(), abs_diff(column, producer_location_column))
                != hcaches.end())
          || (abs_diff(column, producer_location_column) == 0
              && find(vcaches.begin(), vcaches.end(), abs_diff(row, producer_location_row))
                   != vcaches.end())) {
        ndnHelper.setCsSize(cache_size);
      }
      else {
        ndnHelper.setCsSize(0);
      }

      ndnHelper.Install(grid.GetNode(row, column));
    }
  }

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Getting containers for the consumer/producer
  Ptr<Node> producer =
    grid.GetNode(producer_location_row, producer_location_column); // At the center
  NodeContainer consumerNodes;

  for (unsigned i = 0u; i < clients; i++) {
    const uint32_t row = uniformRandomVar->GetInteger(0, rows - 1);
    const uint32_t col = uniformRandomVar->GetInteger(0, columns - 1);
    consumerNodes.Add(grid.GetNode(row, col));
  }

  // Install NDN applications
  static const char prefix[] = "/icarus/static-grid/cache-test/1/";

  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", DoubleValue(1e-3));
  consumerHelper.SetAttribute("MaxSeq", IntegerValue(1));
  consumerHelper.SetAttribute("RetxTimer", TimeValue(Days(1)));

  // Have to install one by one to be able to set start time!
  for (auto consumerNode = consumerNodes.Begin(); consumerNode != consumerNodes.End();
       consumerNode++) {
    auto appContainer = consumerHelper.Install(*consumerNode);
    Time start_time = Seconds(uniformRandomVar->GetValue(0, duration.GetSeconds() - 0.5));
    appContainer.Start(start_time);
  }

  ndn::AppHelper producerHelper("ns3::ndn::Producer");
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", UintegerValue(1024));
  producerHelper.Install(producer);

  //  Calculate and install FIBs
  auto routerHelper = IcarusRouterGridHelper::CreateRouterHelper(routerHelperName, grid);
  routerHelper->addCacheLocations(hcaches, vcaches);
  routerHelper->addRoute(prefix, rows / 2, columns / 2);

  std::ofstream cs_trace_os(outPrefix + "cs-cache.txt", ios_base::trunc);
  IcarusGridTracer grid_tracer(grid, cs_trace_os, prefix);
  grid_tracer.TraceGridCS();
  grid_tracer.TraceGridTx();

  Simulator::Stop(duration);

  Simulator::Run();

  Simulator::Destroy();

  return 0;
}
} // namespace icarus
} // namespace ns3

int
main(int argc, char** argv)
{
  return ns3::icarus::main(argc, argv);
}
