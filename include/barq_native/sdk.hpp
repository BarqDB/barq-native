////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef BARQ_NATIVE_SDK_HPP
#define BARQ_NATIVE_SDK_HPP

#if __has_include(<barq_native/util/config.h>)
#include <barq_native/util/config.h>
#endif

#include <utility>

#include <barq_native/schema.hpp>
#include <barq_native/notifications.hpp>
#include <barq_native/flex_sync.hpp>
#include <barq_native/thread_safe_reference.hpp>
#include <barq_native/rbool.hpp>

#include <barq_native/db.hpp>
#include <barq_native/sync.hpp>
#include <barq_native/client_reset.hpp>
#include <barq_native/link.hpp>
#include <barq_native/macros.hpp>
#include <barq_native/managed_binary.hpp>
#include <barq_native/managed_decimal.hpp>
#include <barq_native/managed_dictionary.hpp>
#include <barq_native/managed_list.hpp>
#include <barq_native/managed_mixed.hpp>
#include <barq_native/managed_numeric.hpp>
#include <barq_native/managed_objectid.hpp>
#include <barq_native/managed_primary_key.hpp>
#include <barq_native/managed_indexed.hpp>
#include <barq_native/managed_vector.hpp>
#include <barq_native/managed_set.hpp>
#include <barq_native/managed_string.hpp>
#include <barq_native/managed_timestamp.hpp>
#include <barq_native/managed_uuid.hpp>
#include <barq_native/observation.hpp>
#include <barq_native/results.hpp>

#endif /* BARQ_NATIVE_SDK_HPP */
