﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#pragma once
#include <aws/greengrassv2/GreengrassV2_EXPORTS.h>
#include <aws/core/utils/memory/stl/AWSString.h>

namespace Aws
{
namespace GreengrassV2
{
namespace Model
{
  enum class IoTJobExecutionFailureType
  {
    NOT_SET,
    FAILED,
    REJECTED,
    TIMED_OUT,
    ALL
  };

namespace IoTJobExecutionFailureTypeMapper
{
AWS_GREENGRASSV2_API IoTJobExecutionFailureType GetIoTJobExecutionFailureTypeForName(const Aws::String& name);

AWS_GREENGRASSV2_API Aws::String GetNameForIoTJobExecutionFailureType(IoTJobExecutionFailureType value);
} // namespace IoTJobExecutionFailureTypeMapper
} // namespace Model
} // namespace GreengrassV2
} // namespace Aws
