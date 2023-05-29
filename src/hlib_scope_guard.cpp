// ==========================================================================================================
// THIS SOFTWARE IS CONFIDENTIAL AND PROPRIETARY PROPERTY OF ACTIVEVIDEO NETWORKS LLC OR ITS AFFILIATES.
//
// Copyright (c) 2021 ActiveVideo Networks LLC. All rights reserved.
//
// ActiveVideo, AppCloud, and CloudTV are trademarks or registered trademarks of ActiveVideo.
// All other trademarks are the property of their respective owners.
//
// This software and the accompanying documentation are the confidential and proprietary products of
// ActiveVideo Networks LLC or its affiliates, the use of which is governed by the terms and conditions
// of a separate written agreement entered into by you with ActiveVideo Networks LLC or its affiliates.
// The software and accompanying documentation may be covered by one or more U.S. or international patents
// and patent applications in addition to being protected by applicable trade secret and copyright laws.
// U.S. Patents are listed at http://www.activevideo.com/patents
// THE SOFTWARE MAY NOT BE COPIED, COMPILED, DECOMPILED, DISTRIBUTED, MODIFIED, OR TRANSFERRED WITHOUT THE
// EXPRESS PRIOR WRITTEN PERMISSION OF ACTIVEVIDEO NETWORKS LLC AND ITS LICENSORS, IF ANY.
//
// ActiveVideo Networks LLC
// 333 W. San Carlos St. Suite 900
// San Jose, CA 95110
// United States
// ==========================================================================================================
#include "hlib/scope_guard.hpp"

using namespace hlib;

//
// Public
//
ScopeGuard::ScopeGuard(std::function<void()> on_exit)
    : m_on_exit(std::move(on_exit))
{
    assert(nullptr != m_on_exit);
}

ScopeGuard::ScopeGuard(std::function<void()> on_enter, std::function<void()> on_exit)
    : ScopeGuard(std::move(on_exit))
{
    assert(nullptr != on_enter);
    on_enter();
}

ScopeGuard::~ScopeGuard()
{
    if (nullptr == m_on_exit) {
        return;
    }

    m_on_exit();
}

void ScopeGuard::clear()
{
    m_on_exit = nullptr;
}

