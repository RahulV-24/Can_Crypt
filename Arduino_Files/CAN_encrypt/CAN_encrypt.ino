#include <SPI.h>
#include <mcp2515.h>
#include <Crypto.h>
#include <AES.h>
#include <string.h>
#include <SpritzCipher.h>

//creating an object of AES128 class
AES128 aes128;

//struct can_frame canMsg;
struct can_frame canMsg1;
struct can_frame canMsg1_1;
struct can_frame canMsg1_2;
struct can_frame canMsg1_3;

// struct can_frame canMsgread;
MCP2515 mcp2515(10);

const uint8_t key[16]={
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F
  };
//mac key
const uint8_t testKey[16] = {
  0x2a, 0x7e, 0x15, 0x16,
  0x28, 0xae, 0xd3, 0xa6,
  0xab, 0xf7, 0x15, 0x88,
  0x09, 0xcf, 0x4f, 0x3c,
};


 
long duration;
int distance;

  void testFunc( byte *msg, byte msgLen, const byte *key, byte keyLen,uint8_t mac[])
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


void Encrypt(uint8_t plaintext[],uint8_t length, uint8_t data[])
{
  uint8_t cypher[16];
   uint8_t mac[8];
  Serial.println();
  Serial.println();
  Serial.print("Before Encryption(Plaintext):");
  Serial.println();
  for(int i=0; i<length; i++){
    if(plaintext[i]==0)
       Serial.print("0");
    Serial.print(plaintext[i], HEX);
   }
   
   //encryption AES-128
  aes128.encryptBlock(cypher,plaintext);//cypher->output block and plaintext->input block
  Serial.println();
  

  Serial.print("After Encryption:");
  for(int j=0;j<sizeof(cypher);j++){
      Serial.print(cypher[j], HEX);
    }
    



  testFunc(cypher, sizeof(cypher), testKey, sizeof(testKey),mac);


  Serial.println();

 // output to be tranferred on can (3 can tranfer would be required)
  memcpy(data, cypher, sizeof(cypher));
  memcpy(&data[sizeof(cypher)], mac, sizeof(mac));
  Serial.print("Data : ");
  for(int i=0; i<24; i++){
    if(data[i]==0)
      Serial.print("0");
    Serial.print(data[i], HEX);
   }

}

void setup() {

    Serial.begin(9600);

  //set AES-128 Key
  aes128.setKey(key,16);
  
  //encrypted data + MAC which will be shared through CAN bus.
  
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();
  

}

void loop() {

    // Random data which has to be shared

    uint8_t  data[24];

  canMsg1.can_id  = 0x0F6;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = random(256);
  canMsg1.data[1] = random(256);
  canMsg1.data[2] = random(256);
  canMsg1.data[3] = random(256);
  canMsg1.data[4] = random(256);
  canMsg1.data[5] = random(256);
  canMsg1.data[6] = random(256);
  canMsg1.data[7] = random(256);
  
  //Plaintext (data which has to be encrypted)

  uint8_t plaintext[]={
     canMsg1.data[0], canMsg1.data[1], canMsg1.data[2], canMsg1.data[3],
     canMsg1.data[4], canMsg1.data[5], canMsg1.data[6], canMsg1.data[7]
  };

  uint8_t  length = sizeof(plaintext)/sizeof(plaintext[0]);
  //calling encrypt function
  Encrypt(plaintext,length,data);
  Serial.println();
  Serial.print("Data recieved for transmission: ");
  for(int i=0; i<24; i++){
    if(data[i]==0)
       Serial.print("0");
    Serial.print(data[i], HEX);
   }

   //3 can messgaes that has to be sent
  canMsg1_1.can_id  = canMsg1.can_id;
  canMsg1_1.can_dlc = 8;
  canMsg1_1.data[0] = data[0];
  canMsg1_1.data[1] = data[1];
  canMsg1_1.data[2] = data[2];
  canMsg1_1.data[3] = data[3];
  canMsg1_1.data[4] = data[4];
  canMsg1_1.data[5] = data[5];
  canMsg1_1.data[6] = data[6];
  canMsg1_1.data[7] = data[7];

  canMsg1_2.can_id  = canMsg1.can_id + 1;
  canMsg1_2.can_dlc = 8;
  canMsg1_2.data[0] = data[8];
  canMsg1_2.data[1] = data[9];
  canMsg1_2.data[2] = data[10];
  canMsg1_2.data[3] = data[11];
  canMsg1_2.data[4] = data[12];
  canMsg1_2.data[5] = data[13];
  canMsg1_2.data[6] = data[14];
  canMsg1_2.data[7] = data[15];

  canMsg1_3.can_id  = canMsg1.can_id + 2;
  canMsg1_3.can_dlc = 8;
  canMsg1_3.data[0] = data[16];
  canMsg1_3.data[1] = data[17];
  canMsg1_3.data[2] = data[18];
  canMsg1_3.data[3] = data[19];
  canMsg1_3.data[4] = data[20];
  canMsg1_3.data[5] = data[21];
  canMsg1_3.data[6] = data[22];
  canMsg1_3.data[7] = data[23];

  mcp2515.sendMessage(&canMsg1_1);
  delay(100);
  mcp2515.sendMessage(&canMsg1_2);
  delay(100);
  mcp2515.sendMessage(&canMsg1_3);
  delay(100);
 Serial.println();
  Serial.println("Messages sent");
 
  delay(10000);
}
