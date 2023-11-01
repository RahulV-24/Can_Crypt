#include <Crypto.h>
#include "SipHash_2_4.h"
#include <AES.h>
#include <string.h>
#include "HexConversionUtils.h"

//key[16] cotain 16 byte key(128 bit) for encryption
const uint8_t key[16]={
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F
  };
//mac key
const uint8_t key2[16] = {
  0x2b, 0x7e, 0x15, 0x16,
  0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88,
  0x09, 0xcf, 0x4f, 0x3c,
};

//const uint8_t  length = sizeof(plaintext)/sizeof(plaintext[0]);
//cypher[16] stores the encrypted text
//uint8_t cypher[16];
//decryptedtext[16] stores decrypted text after decryption
//uint8_t decryptedtext[8];
//mac output

//creating an object of AES128 class
AES128 aes128;



void Encrypt(uint8_t plaintext[],uint8_t length, uint8_t data[])
{
  uint8_t cypher[16];
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
    
 //hash
  char hash[17];
  uint8_t mac[8];

  sipHash.initFromPROGMEM(key2);
  for (int i=0; i<sizeof(cypher);i++) {
    sipHash.updateHash((byte)cypher[i]);
  }
  sipHash.finish(); // result in BigEndian format
  reverse64(sipHash.result); // chage to LittleEndian to match  https://131002.net/siphash/siphash24.c
  hexToAscii(sipHash.result,8,hash,17);
  Serial.println();
  Serial.print("Hash : ");
  //  Serial.println(hash);

 //byte array of hash output
  for(int i = 0; i < sizeof(mac)/sizeof(mac[0]); i++) {
      sscanf(hash + 2*i, "%02x", &mac[i]);
    }

   for(int i=0; i<sizeof(mac); i++){
      if(mac[i]<16)
        Serial.print("0");
      Serial.print(mac[i], HEX);
   }

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


int Decrypt(uint8_t data[],uint8_t decryptedtext[]){
       //Decryption AES-128 

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

  //hash for recieved data
  char hash[17];
  uint8_t mac[8];
  sipHash.initFromPROGMEM(key2);
  for (int i=0; i<sizeof(cypher);i++) {
    sipHash.updateHash((byte)cypher[i]);
  }
  sipHash.finish(); // result in BigEndian format
  reverse64(sipHash.result); // chage to LittleEndian to match  https://131002.net/siphash/siphash24.c
  hexToAscii(sipHash.result,8,hash,17);
  Serial.println();
   // Serial.println(hash);
     //byte array of hash output
  for(int i = 0; i < sizeof(mac)/sizeof(mac[0]); i++) {
      sscanf(hash + 2*i, "%02x", &mac[i]);
    }
  Serial.print("Hash of Data recieved for decryption ");
  for(int i=0; i<sizeof(mac); i++){
    if(mac[i]==0)
      Serial.print("0");
    Serial.print(mac[i], HEX);
   }

     //decrypt
  aes128.decryptBlock(decryptedtext,cypher);
  
  Serial.println();
  Serial.print("Dencrypted Text:");
  for(int i=0; i<8; i++){
    Serial.print(decryptedtext[i], HEX);
  }
  Serial.println();
  Serial.print("MAC recieved from data:");
  for(int i=16; i<24; i++){
    Serial.print(data[i], HEX);
  }
  Serial.println();
    //Authenticating MAC
  for(int i=16;i<24;i++){
    if(mac[i]!=data[i]){
      return -1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  aes128.setKey(key,16);// Setting Key for AES
  uint8_t  data[24]; // data which will go in input of can.

  //plaintext[16] contain the text we need to encrypt
  uint8_t plaintext[]={
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77
    };
  uint8_t  length = sizeof(plaintext)/sizeof(plaintext[0]);
  Encrypt(plaintext,length,data);
  Serial.println();
  Serial.print("Data recieved for transmission: ");
  for(int i=0; i<24; i++){
    Serial.print(data[i], HEX);
   }
/*transmit Data


*/

/*                                   DECRYPTION                         */

  uint8_t  decrypted_data[8];
  //Decryption
  Decrypt(data,decrypted_data);
  if(decrypted_data ==-1){
    Serial.println();
    Serial.print("Data Compromised!!");
  }

  Serial.println();
  Serial.print("After Dencryption:");
  for(int i=0; i<sizeof(decrypted_data); i++){
    Serial.print(decrypted_data[i], HEX);
  }
/* data used by ecu


*/
}

void loop() {
  // put your main code here, to run repeatedly:

}