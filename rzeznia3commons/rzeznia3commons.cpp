// This is the main DLL file.

#include "stdafx.h"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <sstream>

#include "rzeznia3commons.h"

#include "protobuf/player.pb.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace rzeznia3commons {



void RZEZNIA3COMMONS_API Send_Chat(SOCKET socket, ChatStruct const& chat)
{
   Chat proto_chat;

   int bytecount = 0;

   proto_chat.set_from(chat.from);
   proto_chat.set_tekst(chat.text);

//   std::cout << "size after serializing is " << proto_chat.ByteSizeLong() << std::endl;
   size_t siz = proto_chat.ByteSizeLong() + 8;
   char *pkt = new char[siz];
   google::protobuf::io::ArrayOutputStream aos(pkt, siz);
   google::protobuf::io::CodedOutputStream *coded_output = new google::protobuf::io::CodedOutputStream(&aos);
   coded_output->WriteVarint32(rzeznia3commons::MT_chat);
   coded_output->WriteVarint32(proto_chat.ByteSize());
   proto_chat.SerializeToCodedStream(coded_output);

   if ((bytecount = send(socket, pkt, siz, 0)) == -1)
   {
      std::cout << "Error sending data: " << WSAGetLastError();
   }
//    else
//       std::cout << "Sent bytes: " << bytecount;
}

ChatStruct RZEZNIA3COMMONS_API Read_Chat(SOCKET socket, google::protobuf::uint32 siz)
{
   int bytecount;
   Chat chat;
   char *buffer = (char*)malloc(siz + 8);//size of the payload and hdr
                                  //Read the entire buffer including the hdr
   if ((bytecount = recv(socket, (char *)buffer, 8 + siz, MSG_WAITALL)) == -1)
   {
      fprintf(stderr, "Error receiving data %d\n", errno);
   }
   //std::cout << "Second read byte count is " << bytecount << std::endl;
   //Assign ArrayInputStream with enough memory 
   google::protobuf::io::ArrayInputStream ais(buffer, siz + 8);
   google::protobuf::io::CodedInputStream coded_input(&ais);
   //Read an unsigned integer with Varint encoding, truncating to 32 bits.
   google::protobuf::uint32 msg_type;
   coded_input.ReadVarint32(&msg_type);
   coded_input.ReadVarint32(&siz);
   //After the message's length is read, PushLimit() is used to prevent the CodedInputStream 
   //from reading beyond that length.Limits are used when parsing length-delimited 
   //embedded messages
   google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);
   //De-Serialize
   chat.ParseFromCodedStream(&coded_input);
   //Once the embedded message has been parsed, PopLimit() is called to undo the limit
   coded_input.PopLimit(msgLimit);

   ChatStruct chatstruct;

   chatstruct.from = *chat.mutable_from();
   chatstruct.text = *chat.mutable_tekst();

   delete[] buffer;
   return chatstruct;
}

}

