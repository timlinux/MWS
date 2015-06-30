#include "GSM_Module.h"

SoftwareSerial cell(Default_RX, Default_TX);

char incoming_char        = 0;
String Incoming_String    = "";
String Decoded_Data       = "";

bool Module_Waiting_Flag  = 0;
bool String_Flag          = 0;
bool OK_Flag              = 0;
bool CME_Error_Flag       = 0;
bool CMS_Error_Flag       = 0;
bool System_Info_Flag     = 0;
bool GPRS_Info_Flag       = 0;
bool TCP_Info_Flag        = 0;
bool GPRS_Flag            = 0;
bool Call_Ready_Flag      = 0;
bool SMS_Ready_Flag       = 0;
bool Network_OK_Flag      = 0;
bool TCP_Connect_Flag     = 0;
bool TCP_Connecting_Flag  = 0;

int Response_Type         = 0;

GSM_Module::GSM_Module(uint8_t Module_RX, uint8_t Module_TX){
  
    SoftwareSerial cell(Module_RX, Module_TX); // RX, TX
    
}

void GSM_Module::Start(int baud){

    cell.begin(baud);  
  
}

void GSM_Module::Refresh(){

    Module_Waiting_Flag  = 0;
    String_Flag          = 0;
    incoming_char        = 0;
    Incoming_String      = "";  
  
}

bool GSM_Module::String_Received(){

    return String_Flag;  
  
}

bool GSM_Module::Module_Waiting(){

    return Module_Waiting_Flag;  
  
}

void GSM_Module::Listen(){
 
  if(cell.available() >0)
    {
      incoming_char = cell.read();    
      Incoming_String += incoming_char; 
      if (incoming_char == '\n'){
        if((Incoming_String == "\r\n") || (Incoming_String == " \r\n")){
          Incoming_String = ""; 
        }else{ 
          String_Flag = 1; 
        }
      }else if (incoming_char == '>'){
        Module_Waiting_Flag = 1;
      }
    }
   
}

String GSM_Module::Received_String(){

    return Incoming_String;  
  
}

String GSM_Module::Decoded_String(){

    return Decoded_Data;  
  
}

void GSM_Module::Wait(int time_out){
  
  int  loops      = 0;
  int  time_steps = 0;
  bool break_loop = 0;
  
  GSM_Module::Refresh();
  
  while(((time_out<0) || (time_steps < time_out)) && !(break_loop)){
    
    GSM_Module::Listen();
    loops = loops + 1; 
    if(loops > 1000){
      loops = 0;
      time_steps = time_steps + 1; 
    } 
    break_loop = Module_Waiting_Flag || String_Flag; 
    
  }
   
}

/*****************************************************************************************************************

*****************************************************************************************************************/

void GSM_Module::Read_Response(){
 
  OK_Flag          = 0;
  CMS_Error_Flag   = 0;
  CME_Error_Flag   = 0;
  System_Info_Flag = 0;
  GPRS_Info_Flag   = 0;
  TCP_Info_Flag    = 0;
  Response_Type    = 0;
  
  if(Incoming_String.startsWith("OK")){
    
    OK_Flag          = 1;
    Response_Type    = 1;

  }else if(Incoming_String.startsWith("+CME")){

    CME_Error_Flag   = 1;
    Response_Type    = 2;

  }else if(Incoming_String.startsWith("+CMS")){

    CMS_Error_Flag   = 1;
    Response_Type    = 3;

  }else if(Incoming_String.startsWith("+SIND")){

    System_Info_Flag = 1;
    Response_Type    = 4;

  }else if(Incoming_String.startsWith("+CGATT")){

    GPRS_Info_Flag   = 1;
    Response_Type    = 5;

  }else if(Incoming_String.startsWith("+SOCKSTATUS")){

    TCP_Info_Flag    = 1;
    Response_Type    = 6;

  }    
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

String GSM_Module::Decode_Sys_Info(){
  
  int String_Length = Incoming_String.length();
  int System_Code   = -1;
  
  if(String_Length == 53){
    
    System_Code = 10 + 2*(Incoming_String.substring(15,16)).toInt();
    
  }else if(String_Length == 10){
    
    System_Code = (Incoming_String.substring(7,8)).toInt();
    
  }else if(String_Length == 11){
    
    System_Code = (Incoming_String.substring(7,9)).toInt();
    
  } 
  
  if(System_Code == 3){
    
    Call_Ready_Flag = 1;
    
  }else if(System_Code == 4){
    
    SMS_Ready_Flag  = 1;
    
  }else if(System_Code == 11){ 
  
    Network_OK_Flag = 1;
  
  }
  
  return Match_System_Code(System_Code);
}

/*****************************************************************************************************************

*****************************************************************************************************************/

String GSM_Module::Decode_GPRS_Info(){
  
  if(Incoming_String.substring(8,9) == "0"){
  
    GPRS_Flag      = 0;
    return GPRS_Info_0;
    
  }else if(Incoming_String.substring(8,9) == "1"){
  
    GPRS_Flag      = 1;
    return GPRS_Info_1;
    
  }

  return Unknown_code;  
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

String GSM_Module::Decode_TCP_Info(){
  
  if(Incoming_String.substring(18,22) == "0104"){
  
    TCP_Connecting_Flag  = 1;
    TCP_Connect_Flag     = 0;
    return TCP_Info_0;
    
  }else if(Incoming_String.substring(18,22) == "0102"){
  
    TCP_Connect_Flag     = 1;
    TCP_Connecting_Flag  = 0;
    return TCP_Info_1;
    
  }else if(Incoming_String.substring(18,22) == "0100"){
  
    TCP_Connect_Flag     = 0;
    TCP_Connecting_Flag  = 0;
    return TCP_Info_2;
    
  }

  return Unknown_code;  
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Decode_Response(){
  
  Read_Response();
  
  Decoded_Data       = "";
  
  switch(Response_Type){
    
    case 1:
      Decoded_Data = "OK";
      return OK;
      
    case 2:
      Decoded_Data = CME_Error;
      return ERROR_CME;
     
    case 3:
      Decoded_Data = CMS_Error;
      return ERROR_CMS; 
      
    case 4:
      Decoded_Data = Decode_Sys_Info();
      return TYPE_SYS_INFO;
      
    case 5:
      Decoded_Data = Decode_GPRS_Info();
      return TYPE_GPRS_INFO;
      
    case 6:
      Decoded_Data = Decode_TCP_Info();
      return TYPE_TCP_INFO;
    
    default:
      Decoded_Data = Incoming_String;
      return UNKNOWN_RESPONSE;
    
  }
}

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::OK_Received(){

  return OK_Flag;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::Sys_Info_Received(){

  return System_Info_Flag;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::GPRS_Info_Received(){

  return GPRS_Info_Flag;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::SMS_Ready(){

  return SMS_Ready_Flag;  
 
} 

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::GPRS_Ready(){
  
  return GPRS_Flag;  
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

bool GSM_Module::Netwok_OK(){
  
  return Network_OK_Flag;  
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/


bool GSM_Module::Socket_Connecting(){
  
  return TCP_Connecting_Flag;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

String GSM_Module::Match_System_Code(int code){
  
 switch(code){
 
   case 0: 
     return Sys_Codes_0;
     
   case 1: 
     return Sys_Codes_1;
   
   case 3: 
     return Sys_Codes_3;
     
   case 4: 
     return Sys_Codes_4;
     
   case 7: 
     return Sys_Codes_7;

   case 10: 
     return Sys_Codes_10;
     
   case 11: 
     return Sys_Codes_11;
     
   case 12: 
     return Sys_Codes_12;
     
   default: 
     return Unknown_code;
     
 }
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Wait_For_Respond(int Response_Type, bool OK_Needed, bool *Flag, int Time_out){
  
  int Status = 0;
  
  Wait(Time_out);
      
  if(String_Received()){
    
    Status = Decode_Response(); 
    if( Status < 0 ){
      
      return Status;
        
    }else if(Status == Response_Type){
      
      if(OK_Needed){
        
        Wait(1000);
        if(String_Received()){
            
          Status = Decode_Response(); 
          if(Flag) return Status; else return 0;
        }
        
      }
           
      if(Flag) return 1; else return 0;  
      
    }else{
      
      return ERROR_WRONG_RESPONCE_TYPE;
      
    }  
  
  }else if((Response_Type == WAITING_FOR_DATA) && Module_Waiting()){
  
    if(Flag) return 1; else return 0; 
  
  }
  
  return ERROR_RESPONSE_TIMED_OUT;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Query_GPRS(){
  
  Send_AT_Command(AT_CMD_Query_GPRS);
  return Wait_For_Respond(TYPE_GPRS_INFO, 1, &GPRS_Flag , 100);
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Activate_PDP(){
  
  Send_AT_Command(AT_CMD_Activate_PDP);
  return Wait_For_Respond(OK, 0, &OK_Flag, 2000);

  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Configure_TCP(String Host, int Port){
  
  String AT_TCP_CONF = AT_CMD_TCP_Config;
  AT_TCP_CONF.concat('"');
  AT_TCP_CONF.concat("TCP");
  AT_TCP_CONF.concat('"');
  AT_TCP_CONF.concat(',');
  AT_TCP_CONF.concat('"');
  AT_TCP_CONF.concat(Host);
  AT_TCP_CONF.concat('"');
  AT_TCP_CONF.concat(',');
  AT_TCP_CONF.concat(Port);

  Send_AT_Command(AT_TCP_CONF);
  return Wait_For_Respond(OK, 0, &OK_Flag, 500);
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Start_TCP_Connection(){
  
  Send_AT_Command(AT_CMD_Start_TCP);
  return Wait_For_Respond(OK, 0, &OK_Flag, 10);
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Query_Socket_Status(){
  
  TCP_Connect_Flag     = 0;
  TCP_Connecting_Flag  = 0;
  
  Send_AT_Command(AT_CMD_Query_Socket);
  return Wait_For_Respond(TYPE_TCP_INFO, 1, &TCP_Connect_Flag, 10);
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

void GSM_Module::Send_AT_Command(String command){
  
  String AT_Command = "AT+";
  AT_Command.concat(command);
  
  cell.println(AT_Command);
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::ERROR_CODE(int Status, int Error){
  
  return Error+Status;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Send_Data(String Data){
  
 int Status       = 0;
 int ST_length    = Data.length();
 String Send_CMD  = AT_CMD_Send_Data;
 
 Send_CMD.concat(ST_length);
 
 Send_AT_Command(Send_CMD);
 
 Status = Wait_For_Respond(WAITING_FOR_DATA, 0, &Module_Waiting_Flag, 1000);
 
 if(Status == OK){
   
   cell.print(Data);
   cell.write(0x1A);
   
   return Wait_For_Respond(OK, 0, &OK_Flag, 1000);
   
 }
 
 return Status;
 
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::Send_SMS(String Number, String MSG){
  
  int Status = 0;
  
  Send_AT_Command(AT_CMD_SMS_Mode);
  Status = Wait_For_Respond(OK, 0, &OK_Flag, 100);
  
  if( Status == OK ){
    
    String SMS_Number = AT_CMD_SMS_Number;  
    SMS_Number.concat(Number);
    SMS_Number.concat("\"");
    
    Send_AT_Command(SMS_Number);
    Status = Wait_For_Respond(WAITING_FOR_DATA, 0, &Module_Waiting_Flag, 100);
    
    if( Status == OK ){
      
       cell.print(MSG);
       cell.write(0x1A);
   
       Status = Wait_For_Respond(OK, 0, &OK_Flag, 1000);
       if( Status == UNKNOWN_RESPONSE ){
         
          return Wait_For_Respond(OK, 0, &OK_Flag, 1000);
         
       }else{
        
          return Status;
         
       } 
      
    }else{
      
      return ERROR_CODE(Status, ERROR_SMS_NUMBER_NOT_SET);
      
    }
    
  }else{
    
    return ERROR_CODE(Status, ERROR_SMS_MODE_NOT_SELECTED); 
    
  }
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::TCP_Connect(String HOST, int PORT){
  
  int STATUS = 0;
  
  STATUS = Query_GPRS();
  if( STATUS != OK ){
    return ERROR_CODE(STATUS, ERROR_GPRS_NOT_ATTACHED);
  }
  
  STATUS = Activate_PDP();
  if( STATUS != OK ){
    return ERROR_CODE(STATUS, ERROR_PDP_ACTIVATION_FAILED);
  }
  
  STATUS = Configure_TCP(HOST, PORT);
  if( STATUS != OK ){
    return ERROR_CODE(STATUS, ERROR_TCP_CONFIG_FAILED);
  }
 
  STATUS = Start_TCP_Connection();
  if( STATUS != OK ){
    return ERROR_CODE(STATUS, ERROR_TCP_START_FAILED);
  }
  
  do{
    
    STATUS = Query_Socket_Status(); 
    
  }while(Socket_Connecting());
  
  if( STATUS != OK ){
    return ERROR_CODE(STATUS, ERROR_TCP_CONNECTION_FAILED);
  }
  
  return STATUS;
  
}

/*****************************************************************************************************************

*****************************************************************************************************************/

int GSM_Module::POST_Data(String HOST, int PORT, int temp, int hum){
  
  int Status = 0;
  
  Status = TCP_Connect(HOST, PORT);
  
  if(Status == OK){
  
    String POST_Data_Line_5 = POST_Data_Line_5_1;
    POST_Data_Line_5.concat(temp);
    POST_Data_Line_5.concat(POST_Data_Line_5_2);
    POST_Data_Line_5.concat(hum);
    
    String POST_Data = POST_Data_Line_1;
    POST_Data.concat(POST_Data_Line_2);
    POST_Data.concat(HOST);
    POST_Data.concat(POST_Data_Line_3);
    POST_Data.concat(POST_Data_Line_4);
    POST_Data.concat((POST_Data_Line_5.length()-4));
    POST_Data.concat(POST_Data_Line_5);
    
    return Send_Data(POST_Data); 
    
  
  }else{
    
    return Status;
    
  }
  
}
