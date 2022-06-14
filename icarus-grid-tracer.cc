/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Universidade de Vigo
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

#include "icarus-grid-tracer.hpp"
#include "icarus-grid-helper.hpp"

#include "ns3/log-macros-disabled.h"
#include "ns3/log.h"
#include "ns3/ndnSIM-module.h"

NS_LOG_COMPONENT_DEFINE("icarus.IcarusGridTracer");

namespace ns3 {
namespace icarus {

IcarusGridTracer::IcarusGridTracer(const IcarusGridHelper& grid, std::ostream& os,
                                   const std::string& match_prefix) noexcept
  : grid(grid)
  , os(os)
  , name_prefix(match_prefix)
  , stats(grid.getRows(), std::vector<NodeStats>(grid.getColumns()))
{
  NS_LOG_FUNCTION(this << &grid << match_prefix);
}

IcarusGridTracer::~IcarusGridTracer() noexcept
{
  NS_LOG_FUNCTION(this);

  os << "# Row\tCol\tHits\tMisses\tPackets\tBytes" << std::endl;

  for (auto row = 0u; row < stats.size(); row++) {
    for (auto col = 0u; col < stats[row].size(); col++) {
      os << row << '\t' << col << '\t' << stats[row][col].hits << '\t' << stats[row][col].misses
         << '\t' << stats[row][col].txPackets << '\t' << stats[row][col].txBytes << std::endl;
    }
  }
}

void
IcarusGridTracer::TraceGridTx() noexcept
{
  NS_LOG_FUNCTION(this);

  for (auto row = 0u; row < grid.getRows(); row++) {
    for (auto col = 0u; col < grid.getColumns(); col++) {
      TraceNodeTx(row, col);
    }
  }
}

void
IcarusGridTracer::TraceNodeCS(const Ptr<Node>& node, std::size_t row, std::size_t col) noexcept
{
  NS_LOG_FUNCTION(this << node << row << col);

  auto l3proto = node->GetObject<ndn::L3Protocol>();
  auto fwd = l3proto->getForwarder();

  fwd->afterCsHit.connect([=](ndn::Interest interest, ndn::Data) {
    if (name_prefix.isPrefixOf(interest.getName())) {
      stats[row][col].hits++;
    }
  });
  fwd->afterCsMiss.connect([=](ndn::Interest interest) {
    if (name_prefix.isPrefixOf(interest.getName())) {
      stats[row][col].misses++;
    }
  });
}

void
IcarusGridTracer::TraceGridCS() noexcept
{
  NS_LOG_FUNCTION(this);

  for (auto row = 0u; row < grid.getRows(); row++) {
    for (auto col = 0u; col < grid.getColumns(); col++) {
      TraceNodeCS(grid.GetNode(row, col), row, col);
    }
  }
}

void
IcarusGridTracer::TraceNodeTx(std::size_t row, std::size_t col) noexcept
{
  NS_LOG_FUNCTION(this << row << col);

  const auto node = grid.GetNode(row, col);

  for (auto deviceIndex = 0u; deviceIndex != node->GetNDevices(); deviceIndex++) {
    auto device = node->GetDevice(deviceIndex);
    device->TraceConnectWithoutContext("MacTx", MakeBoundCallback(&IcarusGridTracer::macTxTrace,
                                                                  this, row, col));
  }
}

void
IcarusGridTracer::macTxTrace(std::size_t row, std::size_t col, Ptr<const Packet> packet) noexcept
{
  NS_LOG_FUNCTION(this << row << col << packet);

  stats[row][col].txPackets += 1;
  stats[row][col].txBytes += packet->GetSize();
}

void
IcarusGridTracer::macTxTrace(IcarusGridTracer* self, std::size_t row, std::size_t col,
                             Ptr<const Packet> packet) noexcept
{
  NS_LOG_FUNCTION(self << row << col << packet);

  return self->macTxTrace(row, col, packet);
}

}
}