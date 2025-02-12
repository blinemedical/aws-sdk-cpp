﻿/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/core/utils/Outcome.h>
#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/client/CoreErrors.h>
#include <aws/core/client/RetryStrategy.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpResponse.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/core/utils/DNS.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/logging/ErrorMacros.h>

#include <aws/appfabric/AppFabricClient.h>
#include <aws/appfabric/AppFabricErrorMarshaller.h>
#include <aws/appfabric/AppFabricEndpointProvider.h>
#include <aws/appfabric/model/BatchGetUserAccessTasksRequest.h>
#include <aws/appfabric/model/ConnectAppAuthorizationRequest.h>
#include <aws/appfabric/model/CreateAppAuthorizationRequest.h>
#include <aws/appfabric/model/CreateAppBundleRequest.h>
#include <aws/appfabric/model/CreateIngestionRequest.h>
#include <aws/appfabric/model/CreateIngestionDestinationRequest.h>
#include <aws/appfabric/model/DeleteAppAuthorizationRequest.h>
#include <aws/appfabric/model/DeleteAppBundleRequest.h>
#include <aws/appfabric/model/DeleteIngestionRequest.h>
#include <aws/appfabric/model/DeleteIngestionDestinationRequest.h>
#include <aws/appfabric/model/GetAppAuthorizationRequest.h>
#include <aws/appfabric/model/GetAppBundleRequest.h>
#include <aws/appfabric/model/GetIngestionRequest.h>
#include <aws/appfabric/model/GetIngestionDestinationRequest.h>
#include <aws/appfabric/model/ListAppAuthorizationsRequest.h>
#include <aws/appfabric/model/ListAppBundlesRequest.h>
#include <aws/appfabric/model/ListIngestionDestinationsRequest.h>
#include <aws/appfabric/model/ListIngestionsRequest.h>
#include <aws/appfabric/model/ListTagsForResourceRequest.h>
#include <aws/appfabric/model/StartIngestionRequest.h>
#include <aws/appfabric/model/StartUserAccessTasksRequest.h>
#include <aws/appfabric/model/StopIngestionRequest.h>
#include <aws/appfabric/model/TagResourceRequest.h>
#include <aws/appfabric/model/UntagResourceRequest.h>
#include <aws/appfabric/model/UpdateAppAuthorizationRequest.h>
#include <aws/appfabric/model/UpdateIngestionDestinationRequest.h>

using namespace Aws;
using namespace Aws::Auth;
using namespace Aws::Client;
using namespace Aws::AppFabric;
using namespace Aws::AppFabric::Model;
using namespace Aws::Http;
using namespace Aws::Utils::Json;
using ResolveEndpointOutcome = Aws::Endpoint::ResolveEndpointOutcome;

const char* AppFabricClient::SERVICE_NAME = "appfabric";
const char* AppFabricClient::ALLOCATION_TAG = "AppFabricClient";

AppFabricClient::AppFabricClient(const AppFabric::AppFabricClientConfiguration& clientConfiguration,
                                 std::shared_ptr<AppFabricEndpointProviderBase> endpointProvider) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             Aws::MakeShared<DefaultAWSCredentialsProviderChain>(ALLOCATION_TAG),
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
  m_clientConfiguration(clientConfiguration),
  m_executor(clientConfiguration.executor),
  m_endpointProvider(std::move(endpointProvider))
{
  init(m_clientConfiguration);
}

AppFabricClient::AppFabricClient(const AWSCredentials& credentials,
                                 std::shared_ptr<AppFabricEndpointProviderBase> endpointProvider,
                                 const AppFabric::AppFabricClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             Aws::MakeShared<SimpleAWSCredentialsProvider>(ALLOCATION_TAG, credentials),
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
    m_clientConfiguration(clientConfiguration),
    m_executor(clientConfiguration.executor),
    m_endpointProvider(std::move(endpointProvider))
{
  init(m_clientConfiguration);
}

AppFabricClient::AppFabricClient(const std::shared_ptr<AWSCredentialsProvider>& credentialsProvider,
                                 std::shared_ptr<AppFabricEndpointProviderBase> endpointProvider,
                                 const AppFabric::AppFabricClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             credentialsProvider,
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
    m_clientConfiguration(clientConfiguration),
    m_executor(clientConfiguration.executor),
    m_endpointProvider(std::move(endpointProvider))
{
  init(m_clientConfiguration);
}

    /* Legacy constructors due deprecation */
  AppFabricClient::AppFabricClient(const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             Aws::MakeShared<DefaultAWSCredentialsProviderChain>(ALLOCATION_TAG),
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
  m_clientConfiguration(clientConfiguration),
  m_executor(clientConfiguration.executor),
  m_endpointProvider(Aws::MakeShared<AppFabricEndpointProvider>(ALLOCATION_TAG))
{
  init(m_clientConfiguration);
}

AppFabricClient::AppFabricClient(const AWSCredentials& credentials,
                                 const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             Aws::MakeShared<SimpleAWSCredentialsProvider>(ALLOCATION_TAG, credentials),
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
    m_clientConfiguration(clientConfiguration),
    m_executor(clientConfiguration.executor),
    m_endpointProvider(Aws::MakeShared<AppFabricEndpointProvider>(ALLOCATION_TAG))
{
  init(m_clientConfiguration);
}

AppFabricClient::AppFabricClient(const std::shared_ptr<AWSCredentialsProvider>& credentialsProvider,
                                 const Client::ClientConfiguration& clientConfiguration) :
  BASECLASS(clientConfiguration,
            Aws::MakeShared<AWSAuthV4Signer>(ALLOCATION_TAG,
                                             credentialsProvider,
                                             SERVICE_NAME,
                                             Aws::Region::ComputeSignerRegion(clientConfiguration.region)),
            Aws::MakeShared<AppFabricErrorMarshaller>(ALLOCATION_TAG)),
    m_clientConfiguration(clientConfiguration),
    m_executor(clientConfiguration.executor),
    m_endpointProvider(Aws::MakeShared<AppFabricEndpointProvider>(ALLOCATION_TAG))
{
  init(m_clientConfiguration);
}

    /* End of legacy constructors due deprecation */
AppFabricClient::~AppFabricClient()
{
  ShutdownSdkClient(this, -1);
}

std::shared_ptr<AppFabricEndpointProviderBase>& AppFabricClient::accessEndpointProvider()
{
  return m_endpointProvider;
}

void AppFabricClient::init(const AppFabric::AppFabricClientConfiguration& config)
{
  AWSClient::SetServiceClientName("AppFabric");
  AWS_CHECK_PTR(SERVICE_NAME, m_endpointProvider);
  m_endpointProvider->InitBuiltInParameters(config);
}

void AppFabricClient::OverrideEndpoint(const Aws::String& endpoint)
{
  AWS_CHECK_PTR(SERVICE_NAME, m_endpointProvider);
  m_endpointProvider->OverrideEndpoint(endpoint);
}

BatchGetUserAccessTasksOutcome AppFabricClient::BatchGetUserAccessTasks(const BatchGetUserAccessTasksRequest& request) const
{
  AWS_OPERATION_GUARD(BatchGetUserAccessTasks);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, BatchGetUserAccessTasks, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, BatchGetUserAccessTasks, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/useraccess/batchget");
  return BatchGetUserAccessTasksOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

ConnectAppAuthorizationOutcome AppFabricClient::ConnectAppAuthorization(const ConnectAppAuthorizationRequest& request) const
{
  AWS_OPERATION_GUARD(ConnectAppAuthorization);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ConnectAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ConnectAppAuthorization", "Required field: AppBundleIdentifier, is not set");
    return ConnectAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.AppAuthorizationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ConnectAppAuthorization", "Required field: AppAuthorizationIdentifier, is not set");
    return ConnectAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppAuthorizationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ConnectAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppAuthorizationIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/connect");
  return ConnectAppAuthorizationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreateAppAuthorizationOutcome AppFabricClient::CreateAppAuthorization(const CreateAppAuthorizationRequest& request) const
{
  AWS_OPERATION_GUARD(CreateAppAuthorization);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, CreateAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("CreateAppAuthorization", "Required field: AppBundleIdentifier, is not set");
    return CreateAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, CreateAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations");
  return CreateAppAuthorizationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreateAppBundleOutcome AppFabricClient::CreateAppBundle(const CreateAppBundleRequest& request) const
{
  AWS_OPERATION_GUARD(CreateAppBundle);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, CreateAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, CreateAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles");
  return CreateAppBundleOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreateIngestionOutcome AppFabricClient::CreateIngestion(const CreateIngestionRequest& request) const
{
  AWS_OPERATION_GUARD(CreateIngestion);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, CreateIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("CreateIngestion", "Required field: AppBundleIdentifier, is not set");
    return CreateIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, CreateIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions");
  return CreateIngestionOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

CreateIngestionDestinationOutcome AppFabricClient::CreateIngestionDestination(const CreateIngestionDestinationRequest& request) const
{
  AWS_OPERATION_GUARD(CreateIngestionDestination);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, CreateIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("CreateIngestionDestination", "Required field: AppBundleIdentifier, is not set");
    return CreateIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("CreateIngestionDestination", "Required field: IngestionIdentifier, is not set");
    return CreateIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, CreateIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestiondestinations");
  return CreateIngestionDestinationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

DeleteAppAuthorizationOutcome AppFabricClient::DeleteAppAuthorization(const DeleteAppAuthorizationRequest& request) const
{
  AWS_OPERATION_GUARD(DeleteAppAuthorization);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, DeleteAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteAppAuthorization", "Required field: AppBundleIdentifier, is not set");
    return DeleteAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.AppAuthorizationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteAppAuthorization", "Required field: AppAuthorizationIdentifier, is not set");
    return DeleteAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppAuthorizationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, DeleteAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppAuthorizationIdentifier());
  return DeleteAppAuthorizationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_DELETE, Aws::Auth::SIGV4_SIGNER));
}

DeleteAppBundleOutcome AppFabricClient::DeleteAppBundle(const DeleteAppBundleRequest& request) const
{
  AWS_OPERATION_GUARD(DeleteAppBundle);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, DeleteAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteAppBundle", "Required field: AppBundleIdentifier, is not set");
    return DeleteAppBundleOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, DeleteAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  return DeleteAppBundleOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_DELETE, Aws::Auth::SIGV4_SIGNER));
}

DeleteIngestionOutcome AppFabricClient::DeleteIngestion(const DeleteIngestionRequest& request) const
{
  AWS_OPERATION_GUARD(DeleteIngestion);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, DeleteIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteIngestion", "Required field: AppBundleIdentifier, is not set");
    return DeleteIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteIngestion", "Required field: IngestionIdentifier, is not set");
    return DeleteIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, DeleteIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  return DeleteIngestionOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_DELETE, Aws::Auth::SIGV4_SIGNER));
}

DeleteIngestionDestinationOutcome AppFabricClient::DeleteIngestionDestination(const DeleteIngestionDestinationRequest& request) const
{
  AWS_OPERATION_GUARD(DeleteIngestionDestination);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, DeleteIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteIngestionDestination", "Required field: AppBundleIdentifier, is not set");
    return DeleteIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteIngestionDestination", "Required field: IngestionIdentifier, is not set");
    return DeleteIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  if (!request.IngestionDestinationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("DeleteIngestionDestination", "Required field: IngestionDestinationIdentifier, is not set");
    return DeleteIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionDestinationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, DeleteIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestiondestinations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionDestinationIdentifier());
  return DeleteIngestionDestinationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_DELETE, Aws::Auth::SIGV4_SIGNER));
}

GetAppAuthorizationOutcome AppFabricClient::GetAppAuthorization(const GetAppAuthorizationRequest& request) const
{
  AWS_OPERATION_GUARD(GetAppAuthorization);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, GetAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetAppAuthorization", "Required field: AppBundleIdentifier, is not set");
    return GetAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.AppAuthorizationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetAppAuthorization", "Required field: AppAuthorizationIdentifier, is not set");
    return GetAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppAuthorizationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, GetAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppAuthorizationIdentifier());
  return GetAppAuthorizationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

GetAppBundleOutcome AppFabricClient::GetAppBundle(const GetAppBundleRequest& request) const
{
  AWS_OPERATION_GUARD(GetAppBundle);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, GetAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetAppBundle", "Required field: AppBundleIdentifier, is not set");
    return GetAppBundleOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, GetAppBundle, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  return GetAppBundleOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

GetIngestionOutcome AppFabricClient::GetIngestion(const GetIngestionRequest& request) const
{
  AWS_OPERATION_GUARD(GetIngestion);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, GetIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetIngestion", "Required field: AppBundleIdentifier, is not set");
    return GetIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetIngestion", "Required field: IngestionIdentifier, is not set");
    return GetIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, GetIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  return GetIngestionOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

GetIngestionDestinationOutcome AppFabricClient::GetIngestionDestination(const GetIngestionDestinationRequest& request) const
{
  AWS_OPERATION_GUARD(GetIngestionDestination);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, GetIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetIngestionDestination", "Required field: AppBundleIdentifier, is not set");
    return GetIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetIngestionDestination", "Required field: IngestionIdentifier, is not set");
    return GetIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  if (!request.IngestionDestinationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("GetIngestionDestination", "Required field: IngestionDestinationIdentifier, is not set");
    return GetIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionDestinationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, GetIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestiondestinations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionDestinationIdentifier());
  return GetIngestionDestinationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

ListAppAuthorizationsOutcome AppFabricClient::ListAppAuthorizations(const ListAppAuthorizationsRequest& request) const
{
  AWS_OPERATION_GUARD(ListAppAuthorizations);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ListAppAuthorizations, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ListAppAuthorizations", "Required field: AppBundleIdentifier, is not set");
    return ListAppAuthorizationsOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ListAppAuthorizations, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations");
  return ListAppAuthorizationsOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

ListAppBundlesOutcome AppFabricClient::ListAppBundles(const ListAppBundlesRequest& request) const
{
  AWS_OPERATION_GUARD(ListAppBundles);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ListAppBundles, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ListAppBundles, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles");
  return ListAppBundlesOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

ListIngestionDestinationsOutcome AppFabricClient::ListIngestionDestinations(const ListIngestionDestinationsRequest& request) const
{
  AWS_OPERATION_GUARD(ListIngestionDestinations);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ListIngestionDestinations, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ListIngestionDestinations", "Required field: AppBundleIdentifier, is not set");
    return ListIngestionDestinationsOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ListIngestionDestinations", "Required field: IngestionIdentifier, is not set");
    return ListIngestionDestinationsOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ListIngestionDestinations, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestiondestinations");
  return ListIngestionDestinationsOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

ListIngestionsOutcome AppFabricClient::ListIngestions(const ListIngestionsRequest& request) const
{
  AWS_OPERATION_GUARD(ListIngestions);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ListIngestions, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ListIngestions", "Required field: AppBundleIdentifier, is not set");
    return ListIngestionsOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ListIngestions, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions");
  return ListIngestionsOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

ListTagsForResourceOutcome AppFabricClient::ListTagsForResource(const ListTagsForResourceRequest& request) const
{
  AWS_OPERATION_GUARD(ListTagsForResource);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, ListTagsForResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.ResourceArnHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("ListTagsForResource", "Required field: ResourceArn, is not set");
    return ListTagsForResourceOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [ResourceArn]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, ListTagsForResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/tags/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetResourceArn());
  return ListTagsForResourceOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_GET, Aws::Auth::SIGV4_SIGNER));
}

StartIngestionOutcome AppFabricClient::StartIngestion(const StartIngestionRequest& request) const
{
  AWS_OPERATION_GUARD(StartIngestion);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, StartIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("StartIngestion", "Required field: IngestionIdentifier, is not set");
    return StartIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("StartIngestion", "Required field: AppBundleIdentifier, is not set");
    return StartIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, StartIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/start");
  return StartIngestionOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

StartUserAccessTasksOutcome AppFabricClient::StartUserAccessTasks(const StartUserAccessTasksRequest& request) const
{
  AWS_OPERATION_GUARD(StartUserAccessTasks);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, StartUserAccessTasks, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, StartUserAccessTasks, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/useraccess/start");
  return StartUserAccessTasksOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

StopIngestionOutcome AppFabricClient::StopIngestion(const StopIngestionRequest& request) const
{
  AWS_OPERATION_GUARD(StopIngestion);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, StopIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("StopIngestion", "Required field: IngestionIdentifier, is not set");
    return StopIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("StopIngestion", "Required field: AppBundleIdentifier, is not set");
    return StopIngestionOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, StopIngestion, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/stop");
  return StopIngestionOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

TagResourceOutcome AppFabricClient::TagResource(const TagResourceRequest& request) const
{
  AWS_OPERATION_GUARD(TagResource);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, TagResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.ResourceArnHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("TagResource", "Required field: ResourceArn, is not set");
    return TagResourceOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [ResourceArn]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, TagResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/tags/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetResourceArn());
  return TagResourceOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_POST, Aws::Auth::SIGV4_SIGNER));
}

UntagResourceOutcome AppFabricClient::UntagResource(const UntagResourceRequest& request) const
{
  AWS_OPERATION_GUARD(UntagResource);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, UntagResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.ResourceArnHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UntagResource", "Required field: ResourceArn, is not set");
    return UntagResourceOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [ResourceArn]", false));
  }
  if (!request.TagKeysHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UntagResource", "Required field: TagKeys, is not set");
    return UntagResourceOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [TagKeys]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, UntagResource, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/tags/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetResourceArn());
  return UntagResourceOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_DELETE, Aws::Auth::SIGV4_SIGNER));
}

UpdateAppAuthorizationOutcome AppFabricClient::UpdateAppAuthorization(const UpdateAppAuthorizationRequest& request) const
{
  AWS_OPERATION_GUARD(UpdateAppAuthorization);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, UpdateAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UpdateAppAuthorization", "Required field: AppBundleIdentifier, is not set");
    return UpdateAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.AppAuthorizationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UpdateAppAuthorization", "Required field: AppAuthorizationIdentifier, is not set");
    return UpdateAppAuthorizationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppAuthorizationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, UpdateAppAuthorization, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appauthorizations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppAuthorizationIdentifier());
  return UpdateAppAuthorizationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_PATCH, Aws::Auth::SIGV4_SIGNER));
}

UpdateIngestionDestinationOutcome AppFabricClient::UpdateIngestionDestination(const UpdateIngestionDestinationRequest& request) const
{
  AWS_OPERATION_GUARD(UpdateIngestionDestination);
  AWS_OPERATION_CHECK_PTR(m_endpointProvider, UpdateIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE);
  if (!request.AppBundleIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UpdateIngestionDestination", "Required field: AppBundleIdentifier, is not set");
    return UpdateIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [AppBundleIdentifier]", false));
  }
  if (!request.IngestionIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UpdateIngestionDestination", "Required field: IngestionIdentifier, is not set");
    return UpdateIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionIdentifier]", false));
  }
  if (!request.IngestionDestinationIdentifierHasBeenSet())
  {
    AWS_LOGSTREAM_ERROR("UpdateIngestionDestination", "Required field: IngestionDestinationIdentifier, is not set");
    return UpdateIngestionDestinationOutcome(Aws::Client::AWSError<AppFabricErrors>(AppFabricErrors::MISSING_PARAMETER, "MISSING_PARAMETER", "Missing required field [IngestionDestinationIdentifier]", false));
  }
  ResolveEndpointOutcome endpointResolutionOutcome = m_endpointProvider->ResolveEndpoint(request.GetEndpointContextParams());
  AWS_OPERATION_CHECK_SUCCESS(endpointResolutionOutcome, UpdateIngestionDestination, CoreErrors, CoreErrors::ENDPOINT_RESOLUTION_FAILURE, endpointResolutionOutcome.GetError().GetMessage());
  endpointResolutionOutcome.GetResult().AddPathSegments("/appbundles/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetAppBundleIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestions/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionIdentifier());
  endpointResolutionOutcome.GetResult().AddPathSegments("/ingestiondestinations/");
  endpointResolutionOutcome.GetResult().AddPathSegment(request.GetIngestionDestinationIdentifier());
  return UpdateIngestionDestinationOutcome(MakeRequest(request, endpointResolutionOutcome.GetResult(), Aws::Http::HttpMethod::HTTP_PATCH, Aws::Auth::SIGV4_SIGNER));
}

