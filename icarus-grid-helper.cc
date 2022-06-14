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

#include "icarus-grid-helper.hpp"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("icarus.IcarusGridHelper");

namespace ns3 {
namespace icarus {

IcarusGridHelper::IcarusGridHelper(std::size_t rows, std::size_t cols,
                                   PointToPointHelper& p2p) noexcept
  : cols(cols)
  , rows(rows)
  , deviceContainersH(rows * cols)
  , deviceContainersV(rows * cols)
{
  NS_LOG_FUNCTION(this << rows << cols << &p2p);

  nodes.Create(rows * cols);
  for (auto row = 0u; row < rows; row++) {
    for (auto col = 0u; col < cols; col++) {
      // Horizontal link
      deviceContainersH[getIndex(row, col)] =
        p2p.Install(nodes.Get(getIndex(row, col)), nodes.Get(getIndex(row, (col + 1) % cols)));

      // Vertical link
      deviceContainersV[getIndex(row, col)] =
        p2p.Install(nodes.Get(getIndex(row, col)), nodes.Get(getIndex((row + 1) % rows, col)));
    }
  }
}
}
} // namespace icarus
