﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/iotwireless/model/SidewalkAccountInfoWithFingerprint.h>
#include <aws/core/utils/json/JsonSerializer.h>

#include <utility>

using namespace Aws::Utils::Json;
using namespace Aws::Utils;

namespace Aws
{
namespace IoTWireless
{
namespace Model
{

SidewalkAccountInfoWithFingerprint::SidewalkAccountInfoWithFingerprint() : 
    m_amazonIdHasBeenSet(false),
    m_appServerPrivateKeyHasBeenSet(false)
{
}

SidewalkAccountInfoWithFingerprint::SidewalkAccountInfoWithFingerprint(JsonView jsonValue) : 
    m_amazonIdHasBeenSet(false),
    m_appServerPrivateKeyHasBeenSet(false)
{
  *this = jsonValue;
}

SidewalkAccountInfoWithFingerprint& SidewalkAccountInfoWithFingerprint::operator =(JsonView jsonValue)
{
  if(jsonValue.ValueExists("AmazonId"))
  {
    m_amazonId = jsonValue.GetString("AmazonId");

    m_amazonIdHasBeenSet = true;
  }

  if(jsonValue.ValueExists("AppServerPrivateKey"))
  {
    m_appServerPrivateKey = jsonValue.GetString("AppServerPrivateKey");

    m_appServerPrivateKeyHasBeenSet = true;
  }

  return *this;
}

JsonValue SidewalkAccountInfoWithFingerprint::Jsonize() const
{
  JsonValue payload;

  if(m_amazonIdHasBeenSet)
  {
   payload.WithString("AmazonId", m_amazonId);

  }

  if(m_appServerPrivateKeyHasBeenSet)
  {
   payload.WithString("AppServerPrivateKey", m_appServerPrivateKey);

  }

  return payload;
}

} // namespace Model
} // namespace IoTWireless
} // namespace Aws
