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

#ifndef ICARUS_ROUTER_HELPER_HPP
#define ICARUS_ROUTER_HELPER_HPP

#include "ndn-cxx/name.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

namespace ns3 {

class UniformRandomVariable;

namespace icarus {

class IcarusGridHelper;

class IcarusRouterGridHelper {
public:
  virtual ~IcarusRouterGridHelper();

  static std::unique_ptr<IcarusRouterGridHelper>
  CreateRouterHelper(const std::string& algorithm, const IcarusGridHelper& grid);

  void addRoute(const ndn::Name& prefix, std::size_t dstRow, std::size_t dstCol);

  virtual void
  addCacheLocations(const std::vector<std::size_t>& horizontal,
                    const std::vector<std::size_t>& vertical)
  {
  }

protected:
  IcarusRouterGridHelper(const IcarusGridHelper& grid);

  auto pos_dif(std::size_t a, std::size_t b) const noexcept;

  virtual void doAddRoute(const ndn::Name& prefix, std::size_t origRow, std::size_t origCol,
                          std::size_t dstRow, std::size_t dstCol) = 0;

  void addRouteH(const ndn::Name& prefix, std::size_t row, std::size_t origCol, std::size_t dstCol);

  void addRouteV(const ndn::Name& prefix, std::size_t origRow, std::size_t col, std::size_t dstRow);

private:
  const IcarusGridHelper& grid;
  ndn::FibHelper fibHelper;
};

}
}

#endif
