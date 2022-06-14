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

#include "icarus-router-helper.hpp"
#include "icarus-grid-helper.hpp"
#include "ns3/abort.h"
#include "ns3/log-macros-disabled.h"
#include "ns3/log.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"
#include <algorithm>
#include <iterator>
#include <memory>

namespace ns3 {
namespace icarus {

NS_LOG_COMPONENT_DEFINE("icarus.IcarusRouterGridHelper");

namespace {

class OptLocationsRouterGridHelper : public IcarusRouterGridHelper {
public:
  OptLocationsRouterGridHelper(const IcarusGridHelper& grid);
  void addCacheLocations(const std::vector<std::size_t>& horizontal,
                         const std::vector<std::size_t>& vertical) override;

protected:
  virtual void doAddRoute(const ndn::Name& prefix, std::size_t origRow, std::size_t origCol,
                          std::size_t dstRow, std::size_t dstCol) override;

private:
  std::vector<std::size_t> hcaches, vcaches;
};
}

IcarusRouterGridHelper::~IcarusRouterGridHelper()
{
  NS_LOG_FUNCTION(this);
}

std::unique_ptr<IcarusRouterGridHelper>
IcarusRouterGridHelper::CreateRouterHelper(const std::string& algorithm,
                                           const IcarusGridHelper& grid)
{
  NS_LOG_FUNCTION(algorithm << &grid);

  if (algorithm == "OptLocations") {
    return std::make_unique<OptLocationsRouterGridHelper>(grid);
  }

  NS_ABORT_MSG("Not a valid routing algorithm.");

  return nullptr;
}

IcarusRouterGridHelper::IcarusRouterGridHelper(const IcarusGridHelper& grid)
  : grid(grid)
{
  NS_LOG_FUNCTION(this);
}

void
IcarusRouterGridHelper::addRoute(const ndn::Name& prefix, std::size_t dstRow, std::size_t dstCol)
{
  NS_LOG_FUNCTION(this << prefix << dstRow << dstCol);

  for (std::size_t origRow = 0u; origRow < grid.getRows(); origRow++) {
    for (std::size_t origCol = 0u; origCol < grid.getColumns(); origCol++) {
      doAddRoute(prefix, origRow, origCol, dstRow, dstCol);
    }
  }
}

auto
IcarusRouterGridHelper::pos_dif(std::size_t a, std::size_t b) const noexcept
{
  return std::max(a, b) - std::min(a, b);
}

void
IcarusRouterGridHelper::addRouteH(const ndn::Name& prefix, std::size_t row, std::size_t origCol,
                                  std::size_t dstCol)
{
  NS_LOG_FUNCTION(this << prefix << row << origCol << dstCol);

  auto dir = IcarusGridHelper::RIGHT;
  if (dstCol < origCol) { // FIXME: Consider circular routes
    dir = IcarusGridHelper::LEFT;
  }

  const auto device = grid.getDevice(row, origCol, dir);
  auto node = grid.GetNode(row, origCol);
  auto ndn = node->GetObject<ndn::L3Protocol>();
  auto face = ndn->getFaceByNetDevice(device);

  fibHelper.AddRoute(node, prefix, face, 1);
}

void
IcarusRouterGridHelper::addRouteV(const ndn::Name& prefix, std::size_t origRow, std::size_t col,
                                  std::size_t dstRow)
{
  NS_LOG_FUNCTION(this << prefix << origRow << col << dstRow);

  auto dir = IcarusGridHelper::UP;
  if (dstRow < origRow) {
    dir = IcarusGridHelper::DOWN;
  }
  const auto device = grid.getDevice(origRow, col, dir);
  auto node = grid.GetNode(origRow, col);
  auto ndn = node->GetObject<ndn::L3Protocol>();
  auto face = ndn->getFaceByNetDevice(device);

  fibHelper.AddRoute(node, prefix, face, 1);
}

OptLocationsRouterGridHelper::OptLocationsRouterGridHelper(const IcarusGridHelper& grid)
  : IcarusRouterGridHelper(grid)
  , hcaches()
  , vcaches()
{
}

void
OptLocationsRouterGridHelper::addCacheLocations(const std::vector<std::size_t>& horizontal,
                                                const std::vector<std::size_t>& vertical)
{
  // These variables contain distances from the destination
  hcaches = horizontal;
  vcaches = vertical;
}

void
OptLocationsRouterGridHelper::doAddRoute(const ndn::Name& prefix, std::size_t origRow,
                                         std::size_t origCol, std::size_t dstRow,
                                         std::size_t dstCol)
{
  NS_LOG_FUNCTION(this << prefix << origRow << origCol << dstRow << dstCol);

  const size_t vdistance = pos_dif(origRow, dstRow);
  const size_t hdistance = pos_dif(origCol, dstCol);

  const auto best = [](const std::size_t distance,
                       const std::vector<std::size_t>& caches) -> std::size_t {
    std::size_t best = 0;

    for (const std::size_t cache : caches) {
      if (cache <= distance) {
        best = max(best, cache);
      }
    }

    return best;
  };
  // Step 1: Filter out caches that are further than us to the destination
  //         in any axe.
  // Step 2: Find closest cache location to us of the remaining ones
  const std::size_t besth = best(hdistance, hcaches);
  const std::size_t bestv = best(vdistance, vcaches);

  // Step 3: Choose direction according to closest cache location.

  if (dstCol == origCol) {
    return addRouteV(prefix, origRow, origCol, dstRow);
  }
  else if (dstRow == origRow) {
    return addRouteH(prefix, origRow, origCol, dstCol);
  }
  else if (bestv > besth) {
    return addRouteH(prefix, origRow, origCol, dstCol);
  }
  else if (bestv <= besth) {
    return addRouteV(prefix, origRow, origCol, dstRow);
  }

  NS_ABORT_MSG("Unhandled route.");
}

}
}
