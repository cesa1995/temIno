// LIBRERIAS DEFINIDAS-----------------------------------------------------
#include <ESP8266WiFi.h>        // libreria ESP8266
#include <ESP8266WebServer.h>   // libreria Server
#include <RTClib.h>             // libreria RTC
#include <Wire.h>               // libreria Protocolo de Comunicacion I2C
#include <DHT.h>                // librería DHT
//-------------------------------------------------------------------------
String ID_DEFAUL   ="stiotca";
String PASS_DEFAUL ="1234567";
String ID_USER;
String PASS_USER;
String DATAC="";
String DATAH="";

ESP8266WebServer server(80);

// CONTACTORES DEFINIDOS---------------------------------------------------
#define Relay4 5  //D1
#define Relay3 4  //D2
#define Relay2 14 //D5
#define Relay1 12 //D6 

//  Orignalmente 
//  Relay   1  2  3  4
//  Pin     5  4  14 12
//
//  Con etiqueta invertida
//  Relay   4  3  2  1
//  Pin     5  4  14 12
//
//#define Relay5 13 //D7
//#define Relay6 15 //D8

#define BOTON 16 // DO
const int  DHT_pin=10;//pin DHT // 13 D7
/*
#define D0  16 
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5  14
#define D6  12
#define D7  13
#define D8  15
#define D9  3
#define D10 1
#define S2  9
#define S3  10
*/
//-------------------------------------------------------------------------

// CONSTANTES DEFINIDAD----------------------------------------------------
RTC_DS1307 RTC;                        // para RTC
#define ADD 0x50                       // DIRECCION DE LA MEMORIA I2C
DHT dht(DHT_pin,DHT11,15);            // para DHT pin y tipo
const int ADD_CLAVE=96;                // Posicion de la clave  para 4 temp.
const int ADD_H=120;                   // posicion humedad 4 temp.
//-------------------------------------------------------------------------
// HORAS PROGRAMADAS ---------------------  // HH , MM, SS //  SOLO SE NECESITA 1
int HoraActual[7];
//int TON[3]; 
//int TOFF[3];
//int TINICIO[3]; 
//int TFIN[3];
int N_CONTAC;
int SE_AN=0;
//------------------------------------------------>
//-------------------HUMEDAD--------------
int HINICIO;
int HFINAL;
int ESTADOH=0;
int AT ;
int RH=0; 
int TP=0;
int CH[4]= { 0 , 0 , 0 ,0}; // 4 relay
//----------------------------------------

// VARIABLES AUXILIARES FIJAS-----------------
int TAux_Max[3]= {23,59,59};
int TAux_Min[3]= {0,0,0};
float TAux_Max_m;
float TAux_Min_m;
//----------------------------------------

// HORAS BASE MINUTOS---------------------
float HoraActual_m;
float T1[4];
float T2[4];
float T3[4];
float T4[4];

float H1[4];
float H2[4];
float H3[4];
float H4[4];

float TIME1[4];
float TIME2[4];
float TIME3[4];
float TIME4[4];

int estadoh;
int estadot;
int estadotem;

int VALC=0;
int VALF=0;
//---------------------------------------- 
// FUNCIONES DISEÑADAS POR EL PROGRAMADOR----------------------------------------------------------------------------------------
float OPEN_EEPROM(int ADDD,unsigned int POS);
void SAVE_EEPROM(int ADDD, int T[24],unsigned int POS);
float Conv_min(int Hora[3]);
byte READ_EEPROM_BIT( int ADD_DISP, unsigned int ADD_DATA );
void WRITE_EEPROM( int ADD_DISP, unsigned int ADD_DATA, byte* DATA, byte Long );
void Temp_Relay(float T_Hora, float T_ON, float T_OFF, float T_INICIO, float T_FIN, float T_MAX, float T_MIN, float N_Contactor,int HR_ON, int HR_OFF/*, int Humedad, int ESTADO*/);
void Act_Relay(float Ton,float Toff,float H, int Relay, float Tmin, float Tmax);
//void GUARDAD_TIEMPOS_Vec_Aux(int T1[3],int T2[3],int T3[3],int T4[3],int DIR_M, int DIR_D);
void ABRIR_TIEMPOS();
void CLAVE();
void GUARDAR_CLAVE(String id,String pass,int AdresM,int Inicio);
void LEER_CLAVE(int AdresM,unsigned int Inicio);
String arregla_simbolos(String a);
int Comp_GuardarTiempos(String ID, String PASS);
void CONFIG();
void ON_OFF_RELAY(int Numero, byte estado);
//void Act_Relay_Humedad(int HRon,int HRoff, int H, int Relay);
//--------------------------------------------------------------------------------------------------------------------------------

int boton=0; //boton de configuracion
String MAC=""; //variables para el nombre del wifi
String NWIFI=""; //variables para el nombre del wifi
char CWIFI[40]; //variables para el nombre del wifi

char diaWeek[][4]={"dom","lun","mar","mie","jue","vie","sab"};

void setup() {
MAC=WiFi.macAddress(); // ingresar a la variable mac la mac del dispositivo wifi
NWIFI="TEMP="+MAC; // unir la variable mac mas el nombre tem
NWIFI.toCharArray(CWIFI,NWIFI.length()+1); //transformar la variable nwifi a char y guardarla en cwifi
delay(100); //--------------------esperar 0,10 segundos 
//WiFi.softAP(CWIFI); // ------------ 
Serial.begin(115200); // iniciar el monitor serial 
//--- iniciar servidor en /config, /clave, /humedad, /informacion
server.on("/config", CONFIG); 
server.on("/clave", CLAVE);
//server.on("/humedad", HUMEDAD);
server.on("/info", INFORMACION);
server.begin();
Serial.println(" "); 
Serial.println("Webserver iniciado...");
// fin de la cofiguracion del servidor
Wire.begin(0,2); // SDA 0 , CLK 2  D3 D4 iniciar comunicacion I2C
delay(100);
RTC.begin(); // inicia RTC (reloj de tiempo real)
delay(500);
Serial.println("CONFIGURACION INICIAL DE LOS TIEMPOS"); 
ABRIR_TIEMPOS();//--------configuracion inicial de tiempos leer la memoria eeprom y ingresarlo a las variables
Serial.println("CLAVE DE USUARIO"); 
LEER_CLAVE(ADD,ADD_CLAVE);
delay(100);
//ABRIR_HUMEDAD(ADD,ADD_H);
delay(100);
TAux_Max_m=Conv_min(TAux_Max);
TAux_Min_m=Conv_min(TAux_Min);
delay(100);
pinMode(BOTON,INPUT);
pinMode(Relay1,OUTPUT);
pinMode(Relay2,OUTPUT);
pinMode(Relay3,OUTPUT);
pinMode(Relay4,OUTPUT);
ON_OFF_RELAY(Relay1,0);
ON_OFF_RELAY(Relay2,0);
ON_OFF_RELAY(Relay3,0);
ON_OFF_RELAY(Relay4,0);
dht.begin(); //inicia DHT22
}

void loop(){
  
boton=digitalRead(BOTON);
while(boton==0){ // colocar en 0
if(VALC==0){
delay(100); //--------------------
WiFi.softAP(CWIFI); //---------------------------------------- Activa la red WIFI
VALC=1;
VALF=0;
}
ON_OFF_RELAY(Relay1,0);
ON_OFF_RELAY(Relay2,0);
ON_OFF_RELAY(Relay3,0);
ON_OFF_RELAY(Relay4,0);
server.handleClient();
delay(2000);
boton=digitalRead(BOTON);
}

if(VALF==0){
delay(100); //-------------------------retardo
WiFi.softAPdisconnect(CWIFI); //-------------------------------- Desactiva el AP 
VALF=1;
VALC=0;
}

update_timeANDdate();


if(SE_AN!=HoraActual[2]){  
RH=0;
TP=0;
    if(ESTADOH==1){
    RH = dht.readHumidity(); 
    TP = dht.readTemperature();
    Serial.println(RH);
    Serial.println(TP);
    }
    else if (ESTADOH==0){
    delay(10);
    }

 
Serial.print(HoraActual[0]);Serial.print(" : ");Serial.print(HoraActual[1]);Serial.print(" : ");Serial.print(HoraActual[2]);Serial.print("  ");Serial.println(diaWeek[HoraActual[3]]);
HoraActual_m=Conv_min(HoraActual);
Temp_Relay(HoraActual_m, TIME1[0], TIME1[1], TIME1[2], TIME1[3], TAux_Max_m, TAux_Min_m, Relay1, HINICIO, HFINAL /*,RH, CH[0]*/);
Temp_Relay(HoraActual_m, TIME2[0], TIME2[1], TIME2[2], TIME2[3], TAux_Max_m, TAux_Min_m, Relay2, HINICIO, HFINAL /*,RH, CH[1]*/);
Temp_Relay(HoraActual_m, TIME3[0], TIME3[1], TIME3[2], TIME3[3], TAux_Max_m, TAux_Min_m, Relay3, HINICIO, HFINAL /*,RH, CH[2]*/);
Temp_Relay(HoraActual_m, TIME4[0], TIME4[1], TIME4[2], TIME4[3], TAux_Max_m, TAux_Min_m, Relay4, HINICIO, HFINAL /*,RH, CH[3]*/);
SE_AN=HoraActual[2];
}
} // end loop

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void update_timeANDdate(){
DateTime now = RTC.now();
HoraActual[0]=now.hour();
HoraActual[1]=now.minute();
HoraActual[2]=now.second();
HoraActual[3]=now.dayOfTheWeek();
HoraActual[4]=now.day();
HoraActual[5]=now.month();
HoraActual[6]=now.year();
}

//---------------------------FUNCION PARA ABRIR LOS TIEMPOS GUARDADOS-------------- 
void ABRIR_TIEMPOS(){
  Serial.println("");
  Serial.println("Estado humedad");
estadoh=OPEN_EEPROM(ADD,240,2);
  Serial.println("");
  Serial.println("Estado tiempo");
estadot=OPEN_EEPROM(ADD,241,2);
  Serial.println("");
  Serial.println("Estado temperatura");
estadotem=OPEN_EEPROM(ADD,242,2);
  Serial.println("");
  Serial.println("Configuracion tiempo # 1");
  delay(100);
TIME1[0]=OPEN_EEPROM(ADD,0,0);
TIME1[1]=OPEN_EEPROM(ADD,6,0);
TIME1[2]=OPEN_EEPROM(ADD,12,0);
TIME1[3]=OPEN_EEPROM(ADD,18,0);  
  Serial.println("");
  Serial.println("Configuracion tiempo # 2");
    delay(100);
TIME2[0]=OPEN_EEPROM(ADD,24,0);
TIME2[1]=OPEN_EEPROM(ADD,30,0);
TIME2[2]=OPEN_EEPROM(ADD,36,0);
TIME2[3]=OPEN_EEPROM(ADD,42,0);  
  Serial.println("");
  Serial.println("Configuracion tiempo # 3");
    delay(100);
TIME3[0]=OPEN_EEPROM(ADD,48,0);
TIME3[1]=OPEN_EEPROM(ADD,54,0);
TIME3[2]=OPEN_EEPROM(ADD,60,0);
TIME3[3]=OPEN_EEPROM(ADD,66,0);  
  Serial.println("");
  Serial.println("Configuracion tiempo # 4");
    delay(100);
TIME4[0]=OPEN_EEPROM(ADD,72,0);
TIME4[1]=OPEN_EEPROM(ADD,78,0);
TIME4[2]=OPEN_EEPROM(ADD,84,0);
TIME4[3]=OPEN_EEPROM(ADD,90,0);  
  Serial.println("");
  Serial.println("Configuracion hum # 1");
  delay(100);
H1[0]=OPEN_EEPROM(ADD,192,1);
H1[1]=OPEN_EEPROM(ADD,195,1);
H1[2]=OPEN_EEPROM(ADD,96,0);
H1[3]=OPEN_EEPROM(ADD,102,0);
  Serial.println("");
  Serial.println("Configuracion hum # 2");
  delay(100);
H2[0]=OPEN_EEPROM(ADD,198,1);
H2[1]=OPEN_EEPROM(ADD,201,1);
H2[2]=OPEN_EEPROM(ADD,108,0);
H2[3]=OPEN_EEPROM(ADD,114,0);  
  Serial.println("");
  Serial.println("Configuracion hum # 3");
  delay(100);
H3[0]=OPEN_EEPROM(ADD,204,1);
H3[1]=OPEN_EEPROM(ADD,207,1);
H3[2]=OPEN_EEPROM(ADD,120,0);
H3[3]=OPEN_EEPROM(ADD,126,0);
  Serial.println("");
  Serial.println("Configuracion hum # 4");
  delay(100);
H4[0]=OPEN_EEPROM(ADD,210,1);
H4[1]=OPEN_EEPROM(ADD,213,1);
H4[2]=OPEN_EEPROM(ADD,132,0);
H4[3]=OPEN_EEPROM(ADD,138,0);
  Serial.println("");
  Serial.println("Configuracion  tem# 1");
  delay(100);
T1[0]=OPEN_EEPROM(ADD,216,1);
T1[1]=OPEN_EEPROM(ADD,219,1);
T1[2]=OPEN_EEPROM(ADD,144,0);
T1[3]=OPEN_EEPROM(ADD,150,0);
  Serial.println("");
  Serial.println("Configuracion tem# 2");
  delay(100);
T2[0]=OPEN_EEPROM(ADD,222,1);
T2[1]=OPEN_EEPROM(ADD,225,1);
T2[2]=OPEN_EEPROM(ADD,156,0);
T2[3]=OPEN_EEPROM(ADD,162,0);  
  Serial.println("");
  Serial.println("Configuracion tem# 3");
  delay(100);
T3[0]=OPEN_EEPROM(ADD,228,1);
T3[1]=OPEN_EEPROM(ADD,231,1);
T3[2]=OPEN_EEPROM(ADD,168,0);
T3[3]=OPEN_EEPROM(ADD,174,0);
  Serial.println("");
  Serial.println("Configuracion tem# 4");
  delay(100);
T4[0]=OPEN_EEPROM(ADD,234,1);
T4[1]=OPEN_EEPROM(ADD,237,1);
T4[2]=OPEN_EEPROM(ADD,180,0);
T4[3]=OPEN_EEPROM(ADD,186,0);
  delay(100);
}

//------------------------------------------------------------------------------------------
// ----------------------------------------------FUNCION PARA LEER DE LA EEPROM----------------------------------------------
 float OPEN_EEPROM(int ADDD,unsigned int POS,int readtype){
  byte aux;
  String data="";

  switch (readtype){
    //leer digitos de 2 en 2
    case 0:{
      int VALOR[3];
      float VALOR_min;
      for(int j=0;j<3;j++){
         for (unsigned int i=POS;i<(POS+2);i++){
         aux=READ_EEPROM_BIT(ADDD,i);
         data=data+String((char)aux); 
      }
        
      VALOR[j]=data.toInt();
      data="";
      aux=0;
      POS=POS+2;
     }
     Serial.print(VALOR[0]);Serial.print(" : ");Serial.print(VALOR[1]);Serial.print(" : ");Serial.println(VALOR[2]);
     VALOR_min=Conv_min(VALOR);
     delay(90);
     return VALOR_min;
    }
    break;
    //leer digitos de 3 en 3 
    case 1:{
      int variable=0;
      for (unsigned int i=POS;i<(POS+3);i++){
         aux=READ_EEPROM_BIT(ADDD,i);
         data=data+String((char)aux); 
      }
      variable=data.toInt();
      data="";
      aux=0;
      Serial.print(variable);Serial.println(" %");
      return variable;
    }
    break;
    //leer digitos de 1 en 1 
    case 2:{
      if(readtype==2){
       int variable=0;
       aux=READ_EEPROM_BIT(ADDD,POS);
       data=data+String((char)aux);
       variable=data.toInt();
       data="";
       aux=0;
       Serial.print("(Estado=0(apagado) y estado=1(encendido)) Estado=");Serial.println(variable);
       return variable;
      }
    }
     break;
    }
 }
//-------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------FUNCION PARA ESCRIBIR UNA CADENA DE BIT DE LA EEPROM-----------------------------------------
 void WRITE_EEPROM( int ADD_DISP, unsigned int ADD_DATA, byte* DATA, byte Long ) {
  Wire.beginTransmission(ADD_DISP);
  Wire.write((int)(ADD_DATA >> 8)); // MSB
  Wire.write((int)(ADD_DATA & 0xFF)); // LSB
  byte c;
  for ( c = 0; c < Long; c++)
  Wire.write(DATA[c]);
  Wire.endTransmission();
 }
//--------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------- FUNCIOM PARA LEER UN BIT DE LA EEPROM--------------------------------------------------
  byte READ_EEPROM_BIT( int ADD_DISP, unsigned int ADD_DATA ) {
    byte READ_DATA = 0xFF;
    Wire.beginTransmission(ADD_DISP);
    Wire.write((int)(ADD_DATA >> 8)); // MSB
    Wire.write((int)(ADD_DATA & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(ADD_DISP,1);
    if (Wire.available()) READ_DATA = Wire.read();
    return READ_DATA;
  }
//------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------FUNCION PARA EL TEMPORIZADOR-------------------------------------------------------
void Temp_Relay(float T_Hora, float T_ON, float T_OFF, float T_INICIO, float T_FIN, float T_MAX, float T_MIN, float N_Contactor,int HR_ON, int HR_OFF/*, int Humedad, int ESTADO*/){
/*if(ESTADO==0){*/
  if(T_INICIO<T_FIN){
    if(T_Hora>T_INICIO && T_Hora<T_FIN){
    Act_Relay(T_ON,T_OFF,T_Hora, N_Contactor, T_INICIO, T_FIN);
    } // end if(T_Hora>T_MIN && T_Hora<T_MAX)
    else{
    //digitalWrite(N_Contactor,LOW);
    //digitalWrite(N_Contactor,HIGH);
    ON_OFF_RELAY(N_Contactor,0); // apagar relay
    } // else
  } // end if(T_INICIO<T_FIN)

  if(T_INICIO>T_FIN){  
    if(T_Hora>T_MIN && T_Hora<T_FIN){
    Act_Relay(T_ON,T_OFF,T_Hora, N_Contactor, T_MIN, T_FIN);
    } // end if(T_Hora>T_MIN && T_Hora<T_FIN)
    else if(T_Hora>T_INICIO && T_Hora<T_MAX){
    Act_Relay(T_ON,T_OFF,T_Hora, N_Contactor, T_INICIO, T_MAX);
    } // else end if(T_Hora>T_INICIO && T_Hora<T_MAX)
    else {
    //digitalWrite(N_Contactor,LOW);
    //digitalWrite(N_Contactor,HIGH);
    ON_OFF_RELAY(N_Contactor,0); // apagar relay
    } // end else
  
  } // end if(T_INICIO>T_FIN)

  if(T_INICIO==T_FIN){
    ON_OFF_RELAY(N_Contactor,0);// apagar relay
    //digitalWrite(N_Contactor,HIGH);
    //digitalWrite(N_Contactor,LOW);
  } // end if(T_INICIO==T_FIN)
//}//end ESTADO==0

/*else if(ESTADO==1){

if(Humedad<=HR_ON){
    Act_Relay(T_ON,T_OFF,T_Hora, N_Contactor, T_INICIO, T_FIN);
    } // end if(Humedad<HR_ON)

else if (Humedad>=HR_OFF){
    ON_OFF_RELAY(N_Contactor,0); // apagar relay
    } // end if (Humedad>HR_OFF)

} // end ESTADO==1

else{
}*/

} // end Temp_Relay
//----------------------------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------FUNCION PARA CONVERTIR DE FORMATO DE HH MM SS A MINUTOS --------------------
float Conv_min(int Hora[3]){
float Hora_m;
Hora_m=Hora[0]*60+Hora[1]+Hora[2]/60.0;
return Hora_m;
} // end Conv_min(int Hora[3])
//----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------FUNCION DE CONTROL PARA EL FUNCIONAMIENTO DE TON Y TOFF ---------------------
void Act_Relay(float Ton,float Toff,float H, int Relay, float Tmin, float Tmax){
float Tfunc=(H-Tmin);
int N=((Tmax-Tmin)/(Ton+Toff))+1;
int n=0;
int State=0;
while(n<N){

  if(Tfunc>((n*Ton)+(n*Toff))  &&  Tfunc<=(((n+1)*Ton)+(n*Toff))){
    State=1; break;
  } // end (Tfunc>((n*Ton)+(n*Toff))  &&  Tfunc<(((n+1)*Ton)+(n*Toff)))
  if(Tfunc>(((n+1)*Ton)+(n*Toff)) && Tfunc<=((((n+1)*Ton)+((n+1)*Toff)))){
    State=0; break;
  }else{ // end (Tfunc>=(((n+1)*Ton)+(n*Toff)) && Tfunc<((((n+1)*Ton)+((n+1)*Toff))))
    n=n+1;  
  } // end else

} // end while(n<N)
//digitalWrite(Relay,State);
//digitalWrite(Relay,!State);
ON_OFF_RELAY(Relay,State);
} // end Act_Relay(float Ton,float Toff,float H, int Relay, float Tmin, float Tmax)

//----------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------FUNCION LEER CONSOLA Y CONFIGURAR NUEVOS TIEMPOS---------------------------------------------------
   void CONFIG(){  //funcion para cofigurar cualquier parametro del temporizador
    
    int RECONFIG;
    int tconfig = server.arg("tconfig").toInt();
    String getssid = server.arg("ssid");
    String getpass = server.arg("pass");
    String ncontac = server.arg("R");

    getssid=arregla_simbolos(getssid);
    getpass=arregla_simbolos(getpass);
    RECONFIG=Comp_GuardarTiempos(getssid, getpass);

    if(RECONFIG==1){
      server.send ( 200,  "text/list", "CLAVE ACEPTADA" );
      String data="";
      switch(tconfig){
        //configurar humedades y la temperatura programadas en el temporizador
        case 0:{
          char dataC[3];
          int humTemp=server.arg("humTemp").toInt();
          N_CONTAC=ncontac.toInt();
          N_CONTAC=((N_CONTAC-1)*3)+192;
          Serial.print("n_contact humedad");
          Serial.println(N_CONTAC);
          delay(100);

          if(humTemp<10){
            data=data+"00"+String(humTemp,DEC);  
          }else if(humTemp<100){
            data=data+"0"+String(humTemp,DEC);
          }else{
            data=data+String(humTemp,DEC);
          }

          data.toCharArray(dataC, data.length()+1);
          delay(100);
          WRITE_EEPROM(ADD,N_CONTAC,(byte *)dataC,sizeof(dataC) );
          delay(100);
          ABRIR_TIEMPOS();
        }
        break;
        //configurar fecha y hora del reloj del temporizador
        case 1:{
          int hora = server.arg("hora").toInt();
          int minto = server.arg("min").toInt();
          int seg = server.arg("seg").toInt();
          int dia = server.arg("dia").toInt();
          int mes = server.arg("mes").toInt();
          int yeard = server.arg("year").toInt();
     
          RTC.adjust(DateTime(yeard, mes, dia, hora, minto, seg));// ajusta RTC SOLO POR PRIMERA VEZ
          delay(100);
          update_timeANDdate();
        }
        break;
        //configurar horas programadas en el temporizador
        case 2:{ 
          char dataC[6];
          int tiempo[3];       
          tiempo[0]=server.arg("hh").toInt();
          tiempo[1]=server.arg("mm").toInt();
          tiempo[2]=server.arg("ss").toInt();
        
          N_CONTAC=ncontac.toInt();
          N_CONTAC=(N_CONTAC-1)*6;
          Serial.print("n_contact tiempo");
          Serial.println(N_CONTAC);

          for (int i=0;i<3;i++){
            if(tiempo[i]<10){
              data=data+"0"+String(tiempo[i],DEC);  
            }else{
              data=data+String(tiempo[i],DEC);
            }
          }
          data.toCharArray(dataC, data.length()+1);
          delay(100);
          WRITE_EEPROM(ADD,N_CONTAC,(byte *)dataC,sizeof(dataC) );
          delay(100);
          ABRIR_TIEMPOS();
          
        } 
        break;
        // configurar la activacion de las variables en el temporizador
        case 3:{      
          char dataC[1];
          int estadoVar=server.arg("estado").toInt();

          N_CONTAC=ncontac.toInt();
          N_CONTAC=(N_CONTAC-1)+240;
          Serial.print("n_contact de estado");
          Serial.println(N_CONTAC);

          data=data+String(estadoVar,DEC);
          data.toCharArray(dataC, data.length()+1);
          delay(100);
          WRITE_EEPROM(ADD,N_CONTAC,(byte *)dataC,sizeof(dataC) );
          delay(100);
          ABRIR_TIEMPOS();          
        }
        break;
      }      
    }else if(RECONFIG==0){
      server.send ( 180,  "text/list", "CLAVE DENEGADA" );
      Serial.println("Denegada la Modificacion");  
    }   
}
//----------------------------------------------------------------------------------------------------------------------------------------
//-----------------------FUNCION DE COMPARACION DE CLAVE PARA PERMITIR ACTUALIZAR LA EEPROM-----------
int Comp_GuardarTiempos(String ID, String PASS){
         if((ID==ID_DEFAUL && PASS==PASS_DEFAUL) || (ID==ID_USER && PASS==PASS_USER)){
           Serial.println("CLAVE ACEPTADA");
           return 1;
        }
         else{
          Serial.println("CLAVE DENEGADA");
          return 0; 
         }
}
//----------------------------------------------------------------------------------------------------
  String arregla_simbolos(String a){ //cuando pasamos los datos del formulario mediante modo GET los caracteres especiales son reemplazados por codigo UTF-8, esta función reemplaza todos los codigos utf de caracteres especiales por el caracter especial correspondiente, o sea si encuentra un %23 en el string lo reemplazará por un #. 
     a.replace("%C3%A1","á");
     a.replace("%C3%A9","é");
     a.replace("%C3%A","í");
     a.replace("%C3%B3","ó");
     a.replace("%C3%BA","ú");
     a.replace("%21","!");
     a.replace("%23","#");
     a.replace("%24","$");
     a.replace("%25","%");
     a.replace("%26","&");
     a.replace("%2F","/");
     a.replace("%28","(");
     a.replace("%29",")");
     a.replace("%3D","=");
     a.replace("%3F","?");
     a.replace("%27","'");
     a.replace("%C2%BF","¿");
     a.replace("%C2%A1","¡");
     a.replace("%C3%B1","ñ");
     a.replace("%C3%91","Ñ");
     a.replace("+"," ");
     a.replace("%2B","+");
     a.replace("%22","\"");
     return a;
      }
//---------------------------------------------------------------------------------
//--------------------- GUARDAR NUEVA CLAVE DE USUARIO-----------------------------
void CLAVE(){
    String ID = server.arg("id");
    String PASS = server.arg("pass");
    String NEWID = server.arg("newid");
    String NEWPASS = server.arg("newpass");
    ID=arregla_simbolos(ID);
    PASS=arregla_simbolos(PASS);
    NEWID=arregla_simbolos(NEWID);
    NEWPASS=arregla_simbolos(NEWPASS);
  
    if((ID==ID_DEFAUL && PASS==PASS_DEFAUL) || (ID==ID_USER && PASS==PASS_USER)){
    Serial.println("Se Ingreso Una Clave Nueva");
    Serial.println("Clave Ingresada");
    Serial.println(ID);
    Serial.println(PASS);
    Serial.println("Nueva Clave");
    Serial.println(NEWID);
    Serial.println(NEWPASS); 
    server.send ( 200,  "text/list", "ACEPTADO" );
    GUARDAR_CLAVE(NEWID,NEWPASS,ADD,ADD_CLAVE); 
    ID_USER="";
    PASS_USER="";
    LEER_CLAVE(ADD,ADD_CLAVE);
    }
    
    else {
    Serial.println("CLAVE INVALIDA");
    Serial.println("ERROR Clave Ingresada");
    Serial.println(ID);
    Serial.println(PASS);
    Serial.println("ERROR Nueva Clave");
    Serial.println(NEWID);
    Serial.println(NEWPASS); 
    server.send ( 200,  "text/list", "DENEGADO" );
    }
}
//---------------------------------------------------------------------------------
//--------------------- GUARDAR CLAVE EEPROM---------------------------------------
void GUARDAR_CLAVE(String id,String pass,int AdresM,int Inicio){
  String Data=id+pass;
  char DataArray[14];
  Data.toCharArray(DataArray, Data.length()+1);  
  WRITE_EEPROM(AdresM,Inicio,(byte *)DataArray,sizeof(DataArray) );
  delay(100);
}

void LEER_CLAVE(int AdresM,unsigned int Inicio){
byte aux;
String ID_aux="";
String PASS_aux="";
  for (unsigned int i=Inicio;i<(Inicio+7);i++){
   aux=READ_EEPROM_BIT(AdresM,i);
   delay(10);
   ID_aux=ID_aux+String((char)aux); 
  }
  for (unsigned int ii=Inicio+7;ii<(Inicio+14);ii++){
   aux=READ_EEPROM_BIT(AdresM,ii);
   delay(10);
   PASS_aux=PASS_aux+String((char)aux); 
  }
  ID_USER=ID_aux;
  PASS_USER=PASS_aux;
  Serial.println(ID_USER);
  Serial.println(PASS_USER);
  
}

void ON_OFF_RELAY(int Numero, byte estado){
  digitalWrite(Numero,estado);
}

int CONV_HH(float Tmin){
    int HH=0;
    HH=Tmin/60;
    return HH;
  }

int CONV_MM(float Tmin){
  int MM=0;
  int HH = CONV_HH(Tmin);
  MM=(Tmin-(HH*60));
  return MM;
  }

int CONV_SS(float Tmin){  
  float SS=0;
  int enSS=0;
  float decSS=0;
  float aux;
  float comSS=0;
  int HH = CONV_HH(Tmin);
  int MM = CONV_MM(Tmin);
  aux=Tmin-((HH*60.0)+MM);
  while(aux>=comSS){
   SS=SS+0.1; 
   comSS=(SS)/60.0;
  }
  enSS=(int)SS;
  decSS=(float)SS-(int)enSS;
  
  if(enSS>0 && decSS<0.5){
   enSS=(int)SS; 
  }
   else if(enSS>0 && decSS>=0.5){
    enSS=(int)SS+1; 
  }
  return enSS;
}

void INFORMACION(){
      int RECONFIG;
      String getssid = server.arg("id");
      String getpass = server.arg("pass");
      String DATAINFO="";
      getssid=arregla_simbolos(getssid);
      getpass=arregla_simbolos(getpass);
      RECONFIG=Comp_GuardarTiempos(getssid, getpass);
      DATAINFO = CreaString();            

      if(RECONFIG==1){
        server.send ( 200,  "application/json", DATAINFO ); // string de datos
      }
      
      if (RECONFIG==0){
        server.send ( 180,  "text/list", "CLAVE DENEGADA" );
      }
        
  }
  
        String CreaString (){
          String DATA="";
          int TON[12];
          int TOF[12];
          int TIN[36];
          int TFI[36];
          int humTempINI[8];
          int humTempFIN[8];

          //relay 1 
            //tiempo
            TON[0]=CONV_HH(TIME1[0]);
            TON[1]=CONV_MM(TIME1[0]);
            TON[2]=CONV_SS(TIME1[0]);
            TOF[0]=CONV_HH(TIME1[1]);
            TOF[1]=CONV_MM(TIME1[1]);
            TOF[2]=CONV_SS(TIME1[1]);
            TIN[0]=CONV_HH(TIME1[2]);
            TIN[1]=CONV_MM(TIME1[2]);
            TIN[2]=CONV_SS(TIME1[2]);
            TFI[0]=CONV_HH(TIME1[3]);
            TFI[1]=CONV_MM(TIME1[3]);
            TFI[2]=CONV_SS(TIME1[3]);

            //humedad
            humTempINI[0]=H1[0];
            humTempFIN[0]=H1[1];
            TIN[3]=CONV_HH(H1[2]);
            TIN[4]=CONV_MM(H1[2]);
            TIN[5]=CONV_SS(H1[2]);
            TFI[3]=CONV_HH(H1[3]);
            TFI[4]=CONV_MM(H1[3]);
            TFI[5]=CONV_SS(H1[3]);

            //temperatura
            humTempINI[1]=T1[0];
            humTempFIN[1]=T1[1];       
            TIN[6]=CONV_HH(T1[2]);
            TIN[7]=CONV_MM(T1[2]);
            TIN[8]=CONV_SS(T1[2]);
            TFI[6]=CONV_HH(T1[3]);              
            TFI[7]=CONV_SS(T1[3]);              
            TFI[8]=CONV_MM(T1[3]);

          //relay 2 
            //tiempo            
            TON[3]=CONV_HH(TIME2[0]);
            TON[4]=CONV_MM(TIME2[0]);
            TON[5]=CONV_SS(TIME2[0]);
            TOF[3]=CONV_HH(TIME2[1]);
            TOF[4]=CONV_MM(TIME2[1]);
            TOF[5]=CONV_SS(TIME2[1]);
            TIN[9]=CONV_HH(TIME2[2]);
            TIN[10]=CONV_MM(TIME2[2]);
            TIN[11]=CONV_SS(TIME2[2]);
            TFI[9]=CONV_HH(TIME2[3]);
            TFI[10]=CONV_MM(TIME2[3]);
            TFI[11]=CONV_SS(TIME2[3]);

            //humedad
            humTempINI[2]=H2[0];
            humTempFIN[2]=H2[1];
            TIN[12]=CONV_HH(H2[2]);
            TIN[13]=CONV_MM(H2[2]);
            TIN[14]=CONV_SS(H2[2]);
            TFI[12]=CONV_HH(H2[3]);
            TFI[13]=CONV_MM(H2[3]);
            TFI[14]=CONV_SS(H2[3]);

            //temperatura
            humTempINI[3]=T2[0];
            humTempFIN[3]=T2[1];
            TIN[15]=CONV_HH(T2[2]);
            TIN[16]=CONV_MM(T2[2]);
            TIN[17]=CONV_SS(T2[2]);
            TFI[15]=CONV_HH(T2[3]);              
            TFI[16]=CONV_MM(T2[3]);              
            TFI[17]=CONV_SS(T2[3]);

          //relay 3
            //tiempo            
            TON[6]=CONV_HH(TIME3[0]);
            TON[7]=CONV_MM(TIME3[0]);
            TON[8]=CONV_SS(TIME3[0]);
            TOF[6]=CONV_HH(TIME3[1]);
            TOF[7]=CONV_MM(TIME3[1]);
            TOF[8]=CONV_SS(TIME3[1]);
            TIN[18]=CONV_HH(TIME3[2]);
            TIN[19]=CONV_MM(TIME3[2]);
            TIN[20]=CONV_SS(TIME3[2]);
            TFI[18]=CONV_HH(TIME3[3]);
            TFI[19]=CONV_MM(TIME3[3]);
            TFI[20]=CONV_SS(TIME3[3]);

            //humedad
            humTempINI[4]=H3[0];
            humTempFIN[4]=H3[1];
            TIN[21]=CONV_HH(H3[2]);
            TIN[22]=CONV_MM(H3[2]);
            TIN[23]=CONV_SS(H3[2]);
            TFI[21]=CONV_HH(H3[3]);
            TFI[22]=CONV_MM(H3[3]);
            TFI[23]=CONV_SS(H3[3]);

            //temperatura
            humTempINI[5]=T3[0];
            humTempFIN[5]=T3[1];
            TIN[24]=CONV_HH(T3[2]);
            TIN[25]=CONV_MM(T3[2]);
            TIN[26]=CONV_SS(T3[2]);
            TFI[24]=CONV_HH(T3[3]);
            TFI[25]=CONV_MM(T3[3]);
            TFI[26]=CONV_SS(T3[3]);

          //relay 4
            //tiempo
            TON[9]=CONV_HH(TIME4[0]);
            TON[10]=CONV_MM(TIME4[0]);
            TON[11]=CONV_SS(TIME4[0]);
            TOF[9]=CONV_HH(TIME4[1]);
            TOF[10]=CONV_MM(TIME4[1]);
            TOF[11]=CONV_SS(TIME4[1]);
            TIN[27]=CONV_HH(TIME4[2]);
            TIN[27]=CONV_MM(TIME4[2]);
            TIN[29]=CONV_SS(TIME4[2]);
            TFI[27]=CONV_HH(TIME4[3]);
            TFI[28]=CONV_MM(TIME4[3]);
            TFI[29]=CONV_SS(TIME4[3]);

            //humedad
            humTempINI[6]=H4[0];
            humTempFIN[6]=H4[1];
            TIN[30]=CONV_HH(H4[2]);
            TIN[31]=CONV_MM(H4[2]);
            TIN[32]=CONV_SS(H4[2]);
            TFI[30]=CONV_HH(H4[3]);              
            TFI[31]=CONV_SS(H4[3]);              
            TFI[32]=CONV_MM(H4[3]);

            //temperatura
            humTempINI[7]=T4[0];
            humTempFIN[7]=T4[1];
            TIN[33]=CONV_HH(T4[2]);
            TIN[34]=CONV_MM(T4[2]);
            TIN[35]=CONV_SS(T4[2]);
            TFI[33]=CONV_HH(T4[3]);              
            TFI[34]=CONV_MM(T4[3]);
            TFI[35]=CONV_SS(T4[3]);

            //configuracion del reloj de tiempo real 

            String hora = (String) HoraActual[0];
            String min = (String) HoraActual[1];
            String seg = (String) HoraActual[2];
            String diaweek = diaWeek[HoraActual[3]];
            String dia = (String) HoraActual[4];
            String mes = (String) HoraActual[5];
            String ano = (String) HoraActual[6];
            
            DATA="{\"""data\":[""{\"";
            //relay 1
            DATA=DATA+"timein_HH\":\""+TIN[0]+"\",\"""timein_MM\":\""+TIN[1]+"\",\"""timein_SS\":\""+TIN[2]+"\",\"""timefi_HH\":\""+TFI[0]+"\",\"""timefi_MM\":\""+TFI[1]+"\",\"""timefi_SS\":\""+TFI[2]+"\",\"""timeon_HH\":\""+TON[0]+"\",\"""timeon_MM\":\""+TON[1]+"\",\"""timeon_SS\":\""+TON[2]+"\",\"""timeoff_HH\":\""+TOF[0]+"\",\"""timeoff_MM\":\""+TOF[1]+"\",\"""timeoff_SS\":\""+TOF[2]+"\",\"";
            DATA=DATA+"hin_HH\":\""+TIN[3]+"\",\"""hin_MM\":\""+TIN[4]+"\",\"""hin_SS\":\""+TIN[5]+"\",\"""hfi_HH\":\""+TFI[3]+"\",\"""hfi_MM\":\""+TFI[4]+"\",\"""hfi_SS\":\""+TFI[5]+"\",\"""HUMini\":\""+humTempINI[0]+"\",\"""HUMfin\":\""+humTempFIN[0]+"\",\"";
            DATA=DATA+"tempin_HH\":\""+TIN[6]+"\",\"""tempin_MM\":\""+TIN[7]+"\",\"""tempin_SS\":\""+TIN[8]+"\",\"""tempfi_HH\":\""+TFI[6]+"\",\"""tempfi_MM\":\""+TFI[7]+"\",\"""tempfi_SS\":\""+TFI[8]+"\",\"""TEMPini\":\""+humTempINI[1]+"\",\"""TEMPfin\":\""+humTempFIN[1]+"\"},";
            //relay 2
            DATA=DATA+"{\"timein_HH\":\""+TIN[9]+"\",\"""timein_MM\":\""+TIN[10]+"\",\"""timein_SS\":\""+TIN[11]+"\",\"""timefi_HH\":\""+TFI[9]+"\",\"""timefi_MM\":\""+TFI[10]+"\",\"""timefi_SS\":\""+TFI[11]+"\",\"""timeon_HH\":\""+TON[3]+"\",\"""timeon_MM\":\""+TON[4]+"\",\"""timeon_SS\":\""+TON[5]+"\",\"""timeoff_HH\":\""+TOF[3]+"\",\"""timeoff_MM\":\""+TOF[4]+"\",\"""timeoff_SS\":\""+TOF[5]+"\",\"";
            DATA=DATA+"hin_HH\":\""+TIN[12]+"\",\"""hin_MM\":\""+TIN[13]+"\",\"""hin_SS\":\""+TIN[14]+"\",\"""hfi_HH\":\""+TFI[12]+"\",\"""hfi_MM\":\""+TFI[13]+"\",\"""hfi_SS\":\""+TFI[14]+"\",\"""HUMini\":\""+humTempINI[2]+"\",\"""HUMfin\":\""+humTempFIN[2]+"\",\"";
            DATA=DATA+"tempin_HH\":\""+TIN[15]+"\",\"""tempin_MM\":\""+TIN[16]+"\",\"""tempin_SS\":\""+TIN[17]+"\",\"""tempfi_HH\":\""+TFI[15]+"\",\"""tempfi_MM\":\""+TFI[16]+"\",\"""tempfi_SS\":\""+TFI[17]+"\",\"""TEMPini\":\""+humTempINI[3]+"\",\"""TEMPfin\":\""+humTempFIN[3]+"\"},";
            //relay 3
            DATA=DATA+"{\"timein_HH\":\""+TIN[18]+"\",\"""timein_MM\":\""+TIN[19]+"\",\"""timein_SS\":\""+TIN[20]+"\",\"""timefi_HH\":\""+TFI[18]+"\",\"""timefi_MM\":\""+TFI[19]+"\",\"""timefi_SS\":\""+TFI[20]+"\",\"""timeon_HH\":\""+TON[6]+"\",\"""timeon_MM\":\""+TON[7]+"\",\"""timeon_SS\":\""+TON[8]+"\",\"""timeoff_HH\":\""+TOF[6]+"\",\"""timeoff_MM\":\""+TOF[7]+"\",\"""timeoff_SS\":\""+TOF[8]+"\",\"";
            DATA=DATA+"hin_HH\":\""+TIN[21]+"\",\"""hin_MM\":\""+TIN[22]+"\",\"""hin_SS\":\""+TIN[23]+"\",\"""hfi_HH\":\""+TFI[21]+"\",\"""hfi_MM\":\""+TFI[22]+"\",\"""hfi_SS\":\""+TFI[23]+"\",\"""HUMini\":\""+humTempINI[4]+"\",\"""HUMfin\":\""+humTempFIN[4]+"\",\"";
            DATA=DATA+"tempin_HH\":\""+TIN[24]+"\",\"""tempin_MM\":\""+TIN[25]+"\",\"""tempin_SS\":\""+TIN[26]+"\",\"""tempfi_HH\":\""+TFI[24]+"\",\"""tempfi_MM\":\""+TFI[25]+"\",\"""tempfi_SS\":\""+TFI[26]+"\",\"""TEMPini\":\""+humTempINI[5]+"\",\"""TEMPfin\":\""+humTempFIN[5]+"\"},";
            //relay 4
            DATA=DATA+"{\"timein_HH\":\""+TIN[27]+"\",\"""timein_MM\":\""+TIN[28]+"\",\"""timein_SS\":\""+TIN[29]+"\",\"""timefi_HH\":\""+TFI[27]+"\",\"""timefi_MM\":\""+TFI[28]+"\",\"""timefi_SS\":\""+TFI[29]+"\",\"""timeon_HH\":\""+TON[9]+"\",\"""timeon_MM\":\""+TON[10]+"\",\"""timeon_SS\":\""+TON[11]+"\",\"""timeoff_HH\":\""+TOF[9]+"\",\"""timeoff_MM\":\""+TOF[10]+"\",\"""timeoff_SS\":\""+TOF[11]+"\",\"";
            DATA=DATA+"hin_HH\":\""+TIN[30]+"\",\"""hin_MM\":\""+TIN[31]+"\",\"""hin_SS\":\""+TIN[32]+"\",\"""hfi_HH\":\""+TFI[30]+"\",\"""hfi_MM\":\""+TFI[31]+"\",\"""hfi_SS\":\""+TFI[32]+"\",\"""HUMini\":\""+humTempINI[6]+"\",\"""HUMfin\":\""+humTempFIN[6]+"\",\"";
            DATA=DATA+"tempin_HH\":\""+TIN[33]+"\",\"""tempin_MM\":\""+TIN[34]+"\",\"""tempin_SS\":\""+TIN[35]+"\",\"""tempfi_HH\":\""+TFI[33]+"\",\"""tempfi_MM\":\""+TFI[34]+"\",\"""tempfi_SS\":\""+TFI[35]+"\",\"""TEMPini\":\""+humTempINI[7]+"\",\"""TEMPfin\":\""+humTempFIN[7]+"\"}],";
            //reloj de tiempo real
            DATA=DATA+"\"""time\":[{\"""hora\":\""+hora+"\",\"""minutos\":\""+min+"\",\"""segundos\":\""+seg+"\",\"""diaWeek\":\""+diaweek+"\",\"""dia\":\""+dia+"\",\"""mes\":\""+mes+"\",\"""year\":\""+ano+"\"}]}";
            
            
          return DATA;
}

//---------------------------------------------------------------------------------
//--------------------- GUARDAR HUMEDAD-----------------------------
/*void HUMEDAD(){  //esta función se dispara al presionar el botón "setear conexión de la página de configuración
    int RECONFIG;
    String getssid = server.arg("ssid");
    String getpass = server.arg("pass");
    String hon = server.arg("ht");
    String ncontac = server.arg("ncontac");
    getssid=arregla_simbolos(getssid);
    getpass=arregla_simbolos(getpass);
    RECONFIG=Comp_GuardarTiempos(getssid, getpass);

    if(RECONFIG==1){
      server.send ( 200,  "text/list", "HUMEDAD ACEPTADA" );
      tiempo[0]=server.arg("hh").toInt();
      tiempo[1]=server.arg("mm").toInt();
      tiempo[2]=server.arg("ss").toInt();
      humtem[1]=server.arg("hum").toInt();
      N_CONTAC=ncontac.toInt();
      N_CONTAC=((N_CONTAC-1)*24)+96;
      Serial.print("n_contact");
      Serial.println(N_CONTAC);
      delay(100);
      ABRIR_TIEMPOS();
      delay(100);
      GUARDAR_HUMEDAD(humtem,estadoh,ncontac,ADD,ADD_H);
      delay(100);
      ABRIR_HUMEDAD(ADD,ADD_H);  
    }


    if(RECONFIG==0){
      server.send ( 180,  "text/list", "HUMEDAD DENEGADA" );
      //server.send("CLAVE DENEGADA");
    Serial.println("Denegada la Modificacion");  
    }
    
}*/
//----------------------------------------------------------------------------------------------
//----------------------------------------------GUARDAR LOS VALORES DE HUMEDAD-------------------
/*void GUARDAR_HUMEDAD(String humtem,String Estado,String n_cont,int AdresM,int Inicio){
  
  String Data="";
  Data=Estado+n_cont+h_on+h_off;
  char DataArray[6];
  Data.toCharArray(DataArray, Data.length()+1);  
  WRITE_EEPROM(AdresM,Inicio,(byte *)DataArray,sizeof(DataArray) );
  delay(100);
}*/
//------------------------------------------------------------------------------------------------
//----------------ABRIR HUMEDAD ------------------------------------------------------------------------
/*void ABRIR_HUMEDAD(int AdresM,unsigned int Inicio){
byte aux;
String ESTADO_aux="";
String CONT_aux="";
String HINICIO_aux="";
String HFINAL_aux="";
int CONT;
CH[0]= 0;
CH[1]= 0;
CH[2]= 0;
CH[3]= 0;
HINICIO=0;
HFINAL=0;

   aux=READ_EEPROM_BIT(AdresM,Inicio);
   ESTADO_aux=ESTADO_aux+String((char)aux); 
   ESTADOH=ESTADO_aux.toInt();

if(ESTADOH==1){
   aux=READ_EEPROM_BIT(AdresM,(Inicio+1));
   CONT_aux=CONT_aux+String((char)aux); 
   CONT=(CONT_aux.toInt())-1;
   CH[CONT]=1;
  for (unsigned int i=Inicio+2;i<(Inicio+4);i++){
   aux=READ_EEPROM_BIT(AdresM,i);
   HINICIO_aux=HINICIO_aux+String((char)aux); 
  } 
  for (unsigned int ii=Inicio+4;ii<(Inicio+6);ii++){
   aux=READ_EEPROM_BIT(AdresM,ii);
   HFINAL_aux=HFINAL_aux+String((char)aux); 
  }
  HINICIO=HINICIO_aux.toInt();
  HFINAL=HFINAL_aux.toInt();
}
  Serial.println(ESTADOH);
  Serial.print(CH[0]);  Serial.print(CH[1]);  Serial.print(CH[2]);  Serial.println(CH[3]);
  Serial.println(HINICIO);
  Serial.println(HFINAL);
}*/
//--------------------------------------------------------------------------------------------------------

//----------------------FUNCION ON OFF RELAY----------------------------------------

//----------------------------------------------------------------------------------

// ----------------------------------------------------------FUNCION DE CONTROL PARA EL FUNCIONAMIENTO DE HON Y HOFF ---------------------
/*void Act_Relay_Humedad(int HRon,int HRoff, int H, int Relay){
if(H<=HRon){
ON_OFF_RELAY(Relay,1);
//Serial.println("menor");
}

else if(H>=HRoff){
ON_OFF_RELAY(Relay,0);
//Serial.println("mayor");
}

else{
//Serial.println("mantengo estado");
}

}*/ // end Act_Humedad(int HRon,int HRoff, int H, int Relay)
//----------------------------------------------------------------------------------------------------------------------------------------
 
//-------------------------------------------------------------------------------------
/*String Contac_H(){
String Data="";
int CONTACTO= 0+CH[0]*1+CH[1]*2+CH[2]*3+CH[3]*4;

if(CONTACTO>0){
Data="\n\n\n\n Temporizador Configurado por Humedad \n\n # "+String(CONTACTO,DEC)+"\n\n Humedad de Inicio : "+String(HINICIO,DEC)+"\n\n Humedad Final : "+String(HFINAL,DEC);
}

if(CONTACTO==0){
Data="\n\n\n\n Ningun Contactor Esta Configurado con Rangos de Humedad";
}
return Data;
}*/

























