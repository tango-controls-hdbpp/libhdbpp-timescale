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

#ifndef _TEST_HELPERS_HPP
#define _TEST_HELPERS_HPP

#include "AttributeTraits.hpp"
#include "HdbppDefines.hpp"

#include <complex>
#include <experimental/random>
#include <memory>
#include <random>
#include <string>

namespace hdbpp_test
{
namespace psql_conn_test
{
    namespace postgres_db
    {
        // connection strings
        const std::string ConnectionString = "user=postgres password=password host=hdb1";
        const std::string HdbppConnectionString = "user=postgres password=password host=hdb1 dbname=hdbpp_test";
    } // namespace postgres_db
} // namespace psql_conn_test

namespace attr_name
{
    // mock test data
    const std::string TestAttrFQDName = "tango://localhost.server.com:10000/test-domain/test-family/test-member/test";
    const std::string TestAttrFQDNameNoTangoQual =
        "localhost.server.com:10000/test-domain/test-family/test-member/test";
    const std::string TestAttrFQDNameNoDomain = "tango://localhost:10000/test-domain/test-family/test-member/test";

    const std::string TestAttrCs = "new_cs";
    const std::string TestAttrFullAttrName = "test-domain/test-family/test-member/test";
    const std::string TestAttrDomain = "test-domain";
    const std::string TestAttrFamily = "test-family";
    const std::string TestAttrMember = "test-member";
    const std::string TestAttrName = "test";

    const std::string TestAttrTangoHost = "localhost.server.com:10000";
    const std::string TestAttrTangoHostWithDomain = "localhost.server.com:10000";

    // represents the name saved into the database, the combination
    // of (tango://)(TestAttrTangoHostWithDomain)(TestAttrFullAttrName)
    const std::string TestAttrFinalName = "tango://" + TestAttrTangoHostWithDomain + "/" + TestAttrFullAttrName;
} // namespace attr_name

namespace attr_info
{
    const std::string AttrInfoDescription = "Description";
    const std::string AttrInfoLabel = "Label";
    const std::string AttrInfoUnit = "Unit";
    const std::string AttrInfoStandardUnit = "Standard Unit";
    const std::string AttrInfoDisplayUnit = "Display Unit";
    const std::string AttrInfoFormat = "Format";
    const std::string AttrInfoRel = "Rel";
    const std::string AttrInfoAbs = "Abs";
    const std::string AttrInfoPeriod = "Period";
} // namespace attr_info

namespace data_gen
{
    template<typename T>
    std::unique_ptr<std::vector<T>> scalarData()
    {
        std::unique_ptr<std::vector<T>> value = std::make_unique<std::vector<T>>();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> d;
        value->push_back(d(gen));
        return std::move(value);
    }

    template<typename T>
    std::unique_ptr<std::vector<T>> spectrumData(int size)
    {
        std::unique_ptr<std::vector<T>> value = std::make_unique<std::vector<T>>();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> d;

        for (int i = 0; i < size; i++)
            value->push_back(d(gen));

        return std::move(value);
    }

    template<>
    std::unique_ptr<std::vector<bool>> scalarData();
    template<>
    std::unique_ptr<std::vector<uint8_t>> scalarData();
    template<>
    std::unique_ptr<std::vector<std::string>> scalarData();
    template<>
    std::unique_ptr<std::vector<float>> scalarData();
    template<>
    std::unique_ptr<std::vector<double>> scalarData();

    template<>
    std::unique_ptr<std::vector<bool>> spectrumData(int size);
    template<>
    std::unique_ptr<std::vector<uint8_t>> spectrumData(int size);
    template<>
    std::unique_ptr<std::vector<std::string>> spectrumData(int size);
    template<>
    std::unique_ptr<std::vector<float>> spectrumData(int size);
    template<>
    std::unique_ptr<std::vector<double>> spectrumData(int size);

    std::unique_ptr<std::vector<int32_t>> scalarStateData();
    std::unique_ptr<std::vector<int32_t>> spectrumStateData(int size);

    template<typename T>
    std::unique_ptr<std::vector<T>> generateData(const hdbpp::AttributeTraits &traits, bool empty_data)
    {
        if (!empty_data)
        {
            if (traits.isArray())
                return std::move(spectrumData<T>(1024));
            else
                return std::move(scalarData<T>());
        }

        return std::move(std::make_unique<std::vector<T>>());
    }

    std::unique_ptr<std::vector<int32_t>> generateStateData(const hdbpp::AttributeTraits &traits, bool empty_data);
} // namespace data_gen
} // namespace hdbpp_test
#endif // _TEST_HELPERS_HPP