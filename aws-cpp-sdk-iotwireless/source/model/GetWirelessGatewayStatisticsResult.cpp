﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/iotwireless/model/GetWirelessGatewayStatisticsResult.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/AmazonWebServiceResult.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/UnreferencedParam.h>

#include <utility>

using namespace Aws::IoTWireless::Model;
using namespace Aws::Utils::Json;
using namespace Aws::Utils;
using namespace Aws;

GetWirelessGatewayStatisticsResult::GetWirelessGatewayStatisticsResult()
{
}

GetWirelessGatewayStatisticsResult::GetWirelessGatewayStatisticsResult(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  *this = result;
}

GetWirelessGatewayStatisticsResult& GetWirelessGatewayStatisticsResult::operator =(const Aws::AmazonWebServiceResult<JsonValue>& result)
{
  JsonView jsonValue = result.GetPayload().View();
  if(jsonValue.ValueExists("WirelessGatewayId"))
  {
    m_wirelessGatewayId = jsonValue.GetString("WirelessGatewayId");

  }

  if(jsonValue.ValueExists("LastUplinkReceivedAt"))
  {
    m_lastUplinkReceivedAt = jsonValue.GetString("LastUplinkReceivedAt");

  }



  return *this;
}
