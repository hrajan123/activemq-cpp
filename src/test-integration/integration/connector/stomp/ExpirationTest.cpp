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

#include "ExpirationTest.h"
#include <integration/IntegrationCommon.h>

#include <sstream>

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/exceptions/ActiveMQException.h>
#include <decaf/lang/Thread.h>
#include <activemq/connector/stomp/StompConnector.h>
#include <decaf/util/Properties.h>
#include <activemq/transport/TransportFactory.h>
#include <decaf/util/UUID.h>
#include <decaf/util/Properties.h>
#include <decaf/util/StringTokenizer.h>
#include <activemq/connector/ConnectorFactoryMap.h>
#include <decaf/net/SocketFactory.h>
#include <activemq/transport/TransportFactory.h>
#include <decaf/net/Socket.h>
#include <decaf/lang/exceptions/NullPointerException.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/core/ActiveMQConsumer.h>
#include <activemq/core/ActiveMQProducer.h>
#include <decaf/util/StringTokenizer.h>
#include <decaf/lang/Boolean.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>

#include <cms/Connection.h>
#include <cms/MessageConsumer.h>
#include <cms/MessageProducer.h>
#include <cms/MessageListener.h>
#include <cms/Startable.h>
#include <cms/Closeable.h>
#include <cms/MessageListener.h>
#include <cms/ExceptionListener.h>
#include <cms/Topic.h>
#include <cms/Queue.h>
#include <cms/TemporaryTopic.h>
#include <cms/TemporaryQueue.h>
#include <cms/Session.h>
#include <cms/BytesMessage.h>
#include <cms/TextMessage.h>
#include <cms/MapMessage.h>
#include <cms/Session.h>

using namespace activemq::connector::stomp;
using namespace activemq::transport;
using namespace activemq::util;
using namespace std;
using namespace cms;
using namespace activemq;
using namespace activemq::core;
using namespace activemq::util;
using namespace activemq::connector;
using namespace activemq::exceptions;
using namespace decaf::net;
using namespace activemq::transport;
using namespace decaf::util::concurrent;
using namespace decaf::lang;
using namespace decaf::util;

////////////////////////////////////////////////////////////////////////////////
namespace integration {
namespace connector {
namespace stomp{

    class Producer : public decaf::lang::Runnable {
    private:

        cms::Connection* connection;
        cms::Session* session;
        cms::Destination* destination;
        cms::MessageProducer* producer;
        int numMessages;
        long long timeToLive;
        bool disableTimeStamps;
        std::string topic;

    public:

        Producer( std::string topic, int numMessages,
                  long long timeToLive ){

            this->connection = NULL;
            this->session = NULL;
            this->destination = NULL;
            this->producer = NULL;
            this->numMessages = numMessages;
            this->timeToLive = timeToLive;
            this->disableTimeStamps = false;
            this->topic = topic;
        }

        virtual ~Producer(){
            cleanup();
        }

        virtual bool getDisableTimeStamps() const {
            return this->disableTimeStamps;
        }

        virtual void setDisableTimeStamps( bool value ){
            this->disableTimeStamps = value;
        }

        virtual void run() {
            try {
                // Create a ConnectionFactory
                ActiveMQConnectionFactory* connectionFactory =
                    new ActiveMQConnectionFactory(
                        IntegrationCommon::getInstance().getStompURL() );

                // Create a Connection
                connection = connectionFactory->createConnection();
                delete connectionFactory;
                connection->start();

                string sss=connection->getClientID();
                cout << sss << endl;

                session = connection->createSession( Session::AUTO_ACKNOWLEDGE);
                destination = session->createQueue( topic );

                producer = session->createProducer( destination );
                producer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );
                producer->setDisableMessageTimeStamp( disableTimeStamps );

                //unsigned long ttt=getcurt();
                producer->setTimeToLive( 1 );

                // Create the Thread Id String
                string threadIdStr = Integer::toString( Thread::getId() );

                // Create a messages
                string text = (string)"Hello world! from thread " + threadIdStr;

                for( int ix=0; ix<numMessages; ++ix ){
                    TextMessage* message = session->createTextMessage( text );
                    producer->send( message );
                    delete message;
                }

            } catch ( CMSException& e ) {
                e.printStackTrace();
            }
        }

    private:

        void cleanup(){

            // Destroy resources.
            try{
                if( destination != NULL ) delete destination;
            }catch ( CMSException& e ) {}
            destination = NULL;

            try{
                if( producer != NULL ) delete producer;
            }catch ( CMSException& e ) {}
            producer = NULL;

            // Close open resources.
            try{
                if( session != NULL ) session->close();
                if( connection != NULL ) connection->close();
            }catch ( CMSException& e ) {}

            try{
                if( session != NULL ) delete session;
            }catch ( CMSException& e ) {}
            session = NULL;

            try{
                if( connection != NULL ) delete connection;
            }catch ( CMSException& e ) {}
            connection = NULL;
        }

    };

    class Consumer : public cms::MessageListener, public decaf::lang::Runnable {
    private:

        cms::Connection* connection;
        cms::Session* session;
        cms::Destination* destination;
        cms::MessageConsumer* consumer;
        long waitMillis;
        int numReceived;
        std::string topic;

    public:

        Consumer( std::string topic, long waitMillis ){
            connection = NULL;
            session = NULL;
            destination = NULL;
            consumer = NULL;
            this->waitMillis = waitMillis;
            numReceived = 0;
            this->topic = topic;
        }

        virtual ~Consumer(){
            cleanup();
        }

        virtual int getNumReceived() const{
            return numReceived;
        }

        virtual void run(){

            try {

                string user,passwd,sID;
                user="default";
                passwd="";
                sID="lsgID";

                // Create a Connection
                connection = ActiveMQConnectionFactory::createConnection(
                    IntegrationCommon::getInstance().getStompURL(), user, passwd, sID );

                connection->start();

                // Create a Session
                session = connection->createSession( Session::AUTO_ACKNOWLEDGE);

                // Create the destination (Topic or Queue)
                string t = topic + "?consumer.retroactive=true";

                destination = session->createQueue( t );

                consumer = session->createConsumer( destination );

                consumer->setMessageListener( this );

                // Sleep while asynchronous messages come in.
                Thread::sleep( waitMillis );

            } catch (CMSException& e) {
                e.printStackTrace();
            }
        }

        virtual void onMessage( const cms::Message* message ){

            try{
                const TextMessage* textMessage =
                    dynamic_cast< const TextMessage* >( message );
                textMessage->getText();
                numReceived++;
            } catch (CMSException& e) {
                e.printStackTrace();
            }
        }

    private:

        void cleanup(){

            // Destroy resources.
            try{
                if( destination != NULL ) delete destination;
            }catch (CMSException& e) {}
            destination = NULL;

            try{
                if( consumer != NULL ) delete consumer;
            }catch (CMSException& e) {}
            consumer = NULL;

            // Close open resources.
            try{
                if( session != NULL ) session->close();
                if( connection != NULL ) connection->close();
            }catch (CMSException& e) {}

            try{
                if( session != NULL ) delete session;
            }catch (CMSException& e) {}
            session = NULL;

            try{
                if( connection != NULL ) delete connection;
            }catch (CMSException& e) {}
            connection = NULL;
        }
    };

}}}

using namespace std;
using namespace integration;
using namespace integration::connector::stomp;

////////////////////////////////////////////////////////////////////////////////
void ExpirationTest::testExpired() {

    string topic = UUID::randomUUID().toString();
    Producer producer( topic, 1, 1 );
    Thread producerThread( &producer );
    producerThread.start();
    producerThread.join();

    Thread::sleep( 100 );

    Consumer consumer( topic, 2000 );
    Thread consumerThread( &consumer );
    consumerThread.start();
    consumerThread.join();

    Thread::sleep( 100 );

    CPPUNIT_ASSERT_EQUAL( 0, consumer.getNumReceived() );
}

////////////////////////////////////////////////////////////////////////////////
void ExpirationTest::testNotExpired() {

    string topic = UUID::randomUUID().toString();
    Producer producer( topic, 5, 3000 );
    producer.setDisableTimeStamps( true );
    Thread producerThread( &producer );
    producerThread.start();
    producerThread.join();

    Consumer consumer( topic, 3000 );
    Thread consumerThread( &consumer );
    consumerThread.start();
    consumerThread.join();

    CPPUNIT_ASSERT_EQUAL( 5, consumer.getNumReceived() );
}
