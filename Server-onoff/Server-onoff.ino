#include <SPI.h>
#include <UIPEthernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(11555);




int serverStatus = 4;
String readString = "";




void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  
  pinMode(serverStatus, INPUT);
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        
        
        if (readString.length() < 100) {

          //store characters to string 
          readString += c; 
          //Serial.print(c);
        } 
        
        
        
        
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
	  //client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>KERN Server</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>Status serverja:</h1>");
          
          if(digitalRead(serverStatus)) 
            client.println("<p>Server je  <font color=""green"">PRIZGAN</font>!</p>");
          else
            client.println("<p>Server je  <font color=""red"">UGASNJEN</font>!</p>");
          client.println("<br>");   
          client.println("<input type=button value=Prizgi/Ugasni onmousedown=location.href='/?Serveron\'>");; 
          client.println("<p>Po kliku gumba NE osvezuj strani, saj se to steje kot ponoven klik!</p>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    
    if(readString.indexOf("Serveron") >0)//checks for on
          {
            digitalWrite(5, HIGH);  
            delay(500);
            digitalWrite(5, LOW);  
            Serial.println("Led 4 On");
          }
    readString="";
  }
}

