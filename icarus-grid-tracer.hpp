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

#ifndef ICARUS_GRID_TRACER_HPP
#define ICARUS_GRID_TRACER_HPP

#include "ndn-cxx/name.hpp"
#include "ns3/packet.h"
#include <ostream>

namespace ns3 {

class Node;

namespace icarus {

class IcarusGridHelper;

class IcarusGridTracer {
public:
  IcarusGridTracer(const IcarusGridHelper& grid, std::ostream& os,
                   const std::string& match_prefix = "") noexcept;

  ~IcarusGridTracer() noexcept;

  void TraceGridTx() noexcept;
  void TraceNodeCS(const Ptr<Node>& node, std::size_t row, std::size_t col) noexcept;
  void TraceGridCS() noexcept;

private:
  const IcarusGridHelper& grid;
  std::ostream& os;
  ndn::Name name_prefix;
  struct NodeStats {
    std::size_t misses = 0;
    std::size_t hits = 0;
    std::size_t txPackets = 0;
    std::size_t txBytes = 0;
  };
  std::vector<std::vector<NodeStats>> stats;

  void TraceNodeTx(std::size_t row, std::size_t col) noexcept;
  void macTxTrace(std::size_t row, std::size_t col, Ptr<const Packet> packet) noexcept;
  static void macTxTrace(IcarusGridTracer* self, std::size_t row, std::size_t col,
                         Ptr<const Packet> packet) noexcept;
};
}
}

#endif