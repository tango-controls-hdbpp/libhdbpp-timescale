/* Copyright (C) : 2014-2019
   European Synchrotron Radiation Facility
   BP 220, Grenoble 38043, FRANCE

   This file is part of libhdb++timescale.

   libhdb++timescale is free software: you can redistribute it and/or modify
   it under the terms of the Lesser GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libhdb++timescale is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser
   GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with libhdb++timescale.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _HDBPP_TX_FACTORY_HPP
#define _HDBPP_TX_FACTORY_HPP

#include <iostream>

namespace hdbpp
{
template<typename Conn>
class HdbppTxFactory
{
public:
    template<template<typename> class Class, typename... Params>
    Class<Conn> createTx(Params &&... params)
    {
        return Class<Conn>((static_cast<Conn &>(*this)), std::forward<Params>(params)...);
    }
};

} // namespace hdbpp
#endif // _HDBPP_TX_FACTORY_HPP
