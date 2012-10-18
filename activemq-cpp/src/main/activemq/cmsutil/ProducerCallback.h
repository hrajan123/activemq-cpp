/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ACTIVEMQ_CMSUTIL_PRODUCERCALLBACK_H_
#define _ACTIVEMQ_CMSUTIL_PRODUCERCALLBACK_H_

#include <cms/Session.h>
#include <cms/MessageProducer.h>
#include <activemq/util/Config.h>

namespace activemq {
namespace cmsutil {

    /**
     * Callback for sending a message to a CMS destination.
     */
    class AMQCPP_API ProducerCallback {
    public:

        virtual ~ProducerCallback();

        /**
         * Execute an action given a session and producer.
         *
         * @param session
         *          the CMS <code>Session</code>
         * @param producer
         *          the CMS <code>Producer</code>
         *
         * @throws cms::CMSException if thrown by CMS API methods
         */
        virtual void doInCms(cms::Session* session, cms::MessageProducer* producer) = 0;

    };

}}

#endif /*_ACTIVEMQ_CMSUTIL_PRODUCERCALLBACK_H_*/
