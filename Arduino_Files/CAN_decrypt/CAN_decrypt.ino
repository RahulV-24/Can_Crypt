#include <SPI.h>
#include <mcp2515.h>
#include <Crypto.h>
#include <AES.h>
#include <string.h>
#include <SpritzCipher.h>

//key[16] cotain 16 byte key(128 bit) for encryption
const uint8_t key[16]={
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F
  };
//mac key
const byte testKey[16] = {
  0x2a, 0x7e, 0x15, 0x16,
  0x28, 0xae, 0xd3, 0xa6,
  0xab, 0xf7, 0x15, 0x88,
  0x09, 0xcf, 0x4f, 0x3c,
};


  struct can_frame canMsg;
  MCP2515 mcp2515(10);

  //creating an object of AES128 class
  AES128 aes128;
  //data which is recieved through CAN bus
  uint8_t  data[24];
  int count = 0;
  int j=0;
  uint8_t  decryptedtext[8];

//HMAC function
  void testFunc( byte *msg, byte msgLen, const byte *key, byte keyLen, uint8_t mac[])
{
  byte macLen = 8; /* 256-bit but we are using 64 bit(if needed more can be changed from here)*/
  unsigned int i;

  spritz_mac(mac, macLen, msg, msgLen, key, keyLen);
  Serial.println();
  Serial.print("MAC : ");
  for (i = 0; i < macLen; i++) {
    if (mac[i] < 0x10) { /* To print "0F" not "F" */
      Serial.write('0');
    }
    Serial.print(mac[i], HEX);
  }

//   }
  Serial.println();
}

  void Decrypt(uint8_t data[],uint8_t decryptedtext[]){
    //Decryption AES-128 
    uint8_t mac[8];
    uint8_t cypher[16];
    for(int i=0;i<16;i++){
      cypher[i] = data[i];
    }
    Serial.println();
    Serial.print("Cypher of Data recieved for decryption ");
    for(int i=0; i<sizeof(cypher); i++){
      if(cypher[i]==0)
        Serial.print("0");
      Serial.print(cypher[i], HEX);
    }

    Serial.println();
    testFunc(cypher, sizeof(cypher), testKey, sizeof(testKey),mac);
     //decrypt
    aes128.decryptBlock(decryptedtext,cypher);
  
    Serial.println();
    Serial.print("Decrypted Text:");
    for(int i=0; i < 8; i++){
      Serial.print(decryptedtext[i], HEX);
      // Serial.print(" ");
    }
    Serial.println();
    Serial.print("MAC recieved from data:");
    for(int i=16; i<24; i++){
      if(cypher[i]==0)
        Serial.print("0");
      Serial.print(data[i], HEX);
    }
    Serial.println();
    //Authenticating MAC
    if(memcmp(data[16],mac[0],8)==0)
    {
      Serial.print("Data Verified");
      Serial.println();
    }
    else{
      Serial.print("Data Compromised");
      Serial.println();
    }
  }

void setup() {
  Serial.begin(115200);
  aes128.setKey(key,16);// Setting Key for AES

  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
   Serial.println(); 

}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      data[i+j] = canMsg.data[i];
      Serial.print(" ");

    }
      Serial.println(); 

      count++;
      j=j+8;
      Serial.println();

      if(count ==3)
      {
/*                                   DECRYPTION                         */
      Serial.print("Data recieved for decryption: ");
      for(int i=0; i<24; i++){
        if(data[i]==0)
          Serial.print("0");
          Serial.print(data[i], HEX);
        }
           //Decryption
      Decrypt(data,decryptedtext);
      Serial.println();
      Serial.print("After Decryption:");
      for(int i=0; i<8/*sizeof(decrypted_data)*/; i++){
        Serial.print(decryptedtext[i], HEX);
      }

      count =0;
      j=0;
    }
  }
}