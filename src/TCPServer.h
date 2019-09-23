/* X-Cheng LWIP Wrap Module Source
 * Copyright (c) 2018-2028 Gene Kong
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef LIB_FE_LWIP_SRC_TCPSERVER_H_
#define LIB_FE_LWIP_SRC_TCPSERVER_H_

namespace FEmbed {

class TCPServer
{
 public:
    TCPServer();
    virtual ~TCPServer();
};

} /* namespace FEmbed */

#endif /* LIB_FE_LWIP_SRC_TCPSERVER_H_ */