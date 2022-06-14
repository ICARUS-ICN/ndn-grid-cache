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

#ifndef ICARUS_GRID_HELPER_HPP
#define ICARUS_GRID_HELPER_HPP

#include "ns3/node.h"
#include "ns3/point-to-point-helper.h"

namespace ns3 {
namespace icarus {

class IcarusGridHelper {
public:
  enum dir { UP, DOWN, LEFT, RIGHT };

  IcarusGridHelper(std::size_t rows, std::size_t cols, PointToPointHelper& p2p) noexcept;

  Ptr<Node>
  GetNode(std::size_t row, std::size_t col) const noexcept
  {
    return nodes.Get(getIndex(row, col));
  }

  std::size_t
  getRows() const noexcept
  {
    return rows;
  }

  std::size_t
  getColumns() const noexcept
  {
    return cols;
  }

  Ptr<NetDevice>
  getDevice(std::size_t row, std::size_t col, dir direction) const noexcept
  {
    switch (direction) {
    case RIGHT:
      return deviceContainersH[getIndex(row, col)].Get(0);
    case LEFT:
      return deviceContainersH[getIndex((row), (col - 1) % cols)].Get(1);
    case UP:
      return deviceContainersV[getIndex(row, col)].Get(0);
    case DOWN:
      return deviceContainersV[getIndex((row - 1) % rows, col)].Get(1);
    default:
      NS_ASSERT("Impossible direction");
      return nullptr;
    }
  }

  auto
  begin() const
  {
    return nodes.begin();
  }

  auto
  begin()
  {
    return nodes.begin();
  }

  auto
  end() const
  {
    return nodes.end();
  }

  auto
  end()
  {
    return nodes.end();
  }

private:
  const std::size_t cols, rows;
  std::vector<NetDeviceContainer> deviceContainersH, deviceContainersV;
  NodeContainer nodes;

  std::size_t
  getIndex(std::size_t row, std::size_t col) const
  {
    NS_ASSERT(row < rows);
    NS_ASSERT(col < cols);

    return row * cols + col;
  }
};

} // namespace icarus
} // namespace ns3
#endif