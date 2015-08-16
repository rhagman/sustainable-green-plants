import java.io.BufferedWriter;
import java.io.FileWriter;
import processing.serial.*;
import controlP5.*;  //UI

Serial myPort;
ControlP5 cp5;

String outFilename = "out.txt";  // Save everything that is not data
String outData = "data.txt";     // Save the data in a nice format for gnuplot
String toLogFile = "";
String input;
Textfield myTextfield;
int sliderRed, sliderGreen, sliderBlue, pump, slidersprayInterval, slidersprayduration, fan, schedule_value;
int[][] schedule = new int[7][24];
int[][] schedule_settings = new int[9][7]; // 9 different settings, each with 7 values
String[] scheduleDay = {"M","T","W","T","F","S","S"};
int[] scheduleColor = { 0xffff0000,0xff00ff00,0xff0000ff,
                        0xffffff00,0xff00ffff,0xffff00ff,
                        0xffffffff,0xffc0c0c0,0xff424242 };

void setup(){
  for (int i = 0; i < 7; i++) { 
    for (int j = 0; j < 24; j++) {
      schedule[i][j] = 10;  //value is 10 if not specified
    }
  }
  size(400, 800);
  frameRate(10);
  noStroke();
  cp5 = new ControlP5(this);
  myTextfield = cp5 .addTextfield("textInput")
                    .setPosition(20, 20)
                    .setSize(200, 20)
                    .setFocus(true)
                    ;
                    
  myTextfield.setAutoClear(false).keepFocus(true);
  cp5.addButton("submit", 0, 240, 20, 80, 20); //focus,posX,Y,sizeX,Y 
  
  cp5.addSlider("sliderRed")
     .setPosition(20,100)
     .setSize(10,200)
     .setRange(0,255)
     .setValue(127)
     .setLabel("Red")
     .setNumberOfTickMarks(11)
     ;
  cp5.addSlider("sliderGreen")
     .setPosition(80,100)
     .setSize(10,200)
     .setRange(0,255)
     .setValue(127)
     .setLabel("White")
     .setNumberOfTickMarks(11)
     ;
  cp5.addSlider("sliderBlue")
     .setPosition(140,100)
     .setSize(10,200)
     .setRange(0,255)
     .setValue(127)
     .setLabel("Blue")
     .setNumberOfTickMarks(11)
     ;
  cp5.addSlider("slidersprayInterval")
     .setPosition(200,100)
     .setSize(10,200)
     .setRange(0,40)
     .setValue(20)
     .setLabel("interval")
     .setNumberOfTickMarks(9)
     ;
  cp5.addSlider("slidersprayduration")
     .setPosition(260,100)
     .setSize(10,200)
     .setRange(0,10)
     .setValue(2)
     .setLabel("duration")
     .setNumberOfTickMarks(11)
     ;
  cp5.addToggle("pump")
     .setPosition(310,100)
     .setSize(10,10)
     .setLabel("on/off pump")
     .setValue(0)
     .setMode(ControlP5.SWITCH)
     ;
  cp5.addToggle("fan")
     .setPosition(310,140)
     .setSize(10,10)
     .setLabel("on/off fan")
     .setValue(0)
     .setMode(ControlP5.SWITCH)
     ;
  for(int i=0;i<9;i++) {
      cp5.addToggle("bang"+i,200,360+i*30,10,10).setId(i).setLabel("");
      cp5.addTextfield("text"+i,220,355+i*30,110,20).setId(i).setLabel("");
  }  
  for(int i=0;i<7;i++) {
    for(int j=0;j<24;j++){
      if(j<23 && i != 6){
        cp5.addToggle("bangd"+i+"h"+j,20+i*15,360+j*15,10,10).setId(i).setLabel("");
      }
      if(j==23){
        cp5.addToggle("bangd"+i+"h"+j,20+i*15,360+j*15,10,10).setId(i).setLabel(" "+scheduleDay[i]);
      }
      if(i==6 && j!=23){
        cp5.addToggle("bangd"+i+"h"+j,20+i*15,360+j*15,10,10).setId(i).setLabel(""+j);
        cp5.getController("bangd"+i+"h"+j).getCaptionLabel().align(ControlP5.LEFT, ControlP5.RIGHT_OUTSIDE).setPaddingX(15);
      }
    }
  }
  cp5.addButton("settings", 0, 300, 200, 80, 20) //focus,posX,Y,sizeX,Y 
     .setLabel("save settings")
     ;
  cp5.addButton("send", 0, 200, 630, 80, 20) //focus,posX,Y,sizeX,Y 
     .setLabel("send schedule")
     ;
  cp5.addButton("clear_schedule", 0, 200, 670, 80, 20) //focus,posX,Y,sizeX,Y 
     .setLabel("clear schedule")
     ;  
  
  // Serial 
  println(Serial.list()); // List all the available serial ports:
  myPort = new Serial(this, Serial.list()[0], 9600); // Open the port you are using at the rate you want:
  myPort.clear(); // Throw out the first reading, in case we started reading in the middle of a string from the sender.
}

void draw() {
  background(0);
  for(int i=0; i<9; i++){
      stroke(0);
      fill(scheduleColor[i]);     // use color to fill
      rect(340,360+(i*30),10,10);  // draw rectangle
  }
  serialEvent();
}

void serialEvent()
{
  while (myPort.available() > 0) {
    String inBuffer = myPort.readString();   
    if (inBuffer != null) {
      toLogFile = toLogFile + inBuffer;
      print(inBuffer);
      inBuffer = "";
    }
  }

  if(toLogFile.indexOf("\n") != -1 && toLogFile.indexOf("T") == 0)
  {
    String text = toLogFile.substring(0,toLogFile.indexOf("\n")+1); //String containing what we want to save
    toLogFile = toLogFile.substring(toLogFile.indexOf("\n")+1); //Reset the ReadingString
    
    int t, s, r; //Strip the string from all the characters and add a "," between them
    t = text.indexOf("T");
    s = text.indexOf("S");
    r = text.substring(t).indexOf("R") + t; //to avoid error msg from dht11 lib (Read fail)
    text = text.substring(t+1,s) + "," + text.substring(s+1,r) + "," + text.substring(r+1); 
    
    appendTextToFile(outData, text);
  }
  else if(toLogFile.indexOf("\n") != -1 )
  {
    String text = toLogFile.substring(0,toLogFile.indexOf("\n")+1); //String containing what we want to save
    toLogFile = toLogFile.substring(toLogFile.indexOf("\n")+1); //Reset the ReadingString
    
    appendTextToFile(outFilename, text);
  }

}


void submit() {
  input = cp5.get(Textfield.class,"textInput").getText();
  print("Submitted: " + input);
  println();
  myPort.write(input);
  input = "";
}

void settings() {
  schedule_settings[schedule_value][0] = sliderRed;
  schedule_settings[schedule_value][1] = sliderGreen;
  schedule_settings[schedule_value][2] = sliderBlue;
  schedule_settings[schedule_value][3] = pump;
  schedule_settings[schedule_value][4] = slidersprayInterval;
  schedule_settings[schedule_value][5] = slidersprayduration; 
  schedule_settings[schedule_value][6] = fan;
  
  String sText = "";
  for(int i=0;i<7;i++){
    if(i<6) {
      sText = sText + schedule_settings[schedule_value][i] + ",";
    }
    if(i==6) {
      sText = sText + schedule_settings[schedule_value][i];
    }
  }
  ((Textfield)cp5.controller("text"+schedule_value)).setText(sText); 
}

void send(){
 /** Sends two things to the arduino over Seriel. 
  *  First the settings (U), second the hour matrix (I)
  */
  
  String ScheduleText = "";
  int elements = 0;
  int nElements = 9;
  for(int i=0;i<9;i++){
    elements = ScheduleText.length();
    if(i<8) {
      ScheduleText = ScheduleText + cp5.get(Textfield.class,"text"+i).getText() + ".";
      if(elements == ScheduleText.length()-1){
        nElements = nElements - 1;
      }
    }
    if(i==8){
      ScheduleText = ScheduleText + cp5.get(Textfield.class,"text"+i).getText() + "\n";
      if(elements == ScheduleText.length()-1){
        nElements = nElements - 1;
      }
    }
  }
  String SendScheduleText = "U" + nElements + "," + ScheduleText;
  print("Submitted: " + SendScheduleText);
  myPort.write(SendScheduleText);
  
  String ScheduleValues = "";
  nElements = 7;
  for(int i=0;i<7;i++) {
    for(int j=0;j<24;j++){
      elements = ScheduleValues.length();
      if(j<23){
        ScheduleValues = ScheduleValues + schedule[i][j] + ",";
      }
      if(j==23 && i != 6){
        ScheduleValues = ScheduleValues + schedule[i][j] + ".";
        if(schedule[i][j] == 10){
          nElements = nElements - 1;
        }
      }
      if(j==23 && i == 6){
        ScheduleValues = ScheduleValues + schedule[i][j] + "\n";
        if(schedule[i][j] == 10){
          nElements = nElements - 1;
        }
      }
    }
  }
  ScheduleValues = "I" + nElements + "," + ScheduleValues;
  print("Submitted: " + ScheduleValues);
  myPort.write(ScheduleValues);
}

void clear_schedule(){
  for(int i=0;i<7;i++) {
    for(int j=0;j<24;j++){
      cp5.controller("bangd"+i+"h"+j).setValue(0);
      schedule[i][j] = 10;
    }
  }
}

public void controlEvent(ControlEvent theEvent) {
  for(int i=0;i<7;i++) {
    for(int j=0;j<24;j++) {
      if(theEvent.controller().name().equals("bangd"+i+"h"+j)) {
        schedule[i][j] = schedule_value;
        //println(i+","+j+"->"+schedule[i][j]);
        theEvent.controller().setColorActive(scheduleColor[schedule_value]);
      }
    }
  }
  
  for(int i=0;i<9;i++) {
      if(theEvent.controller().name().equals("bang"+i) && theEvent.controller().value()==1) {
        schedule_value = i;
        println(schedule_value);
        theEvent.controller().setColorActive(scheduleColor[schedule_value]); //set the color and set the rest of the toggles to 0
        for(int j=0;j<9;j++) {
          if(i != j)
          cp5.controller("bang"+j).setValue(0);
        }
      }
  }
  
}


/**
 * Appends text to the end of a text file located in the data directory, 
 * creates the file if it does not exist.
 * Can be used for big files with lots of rows, 
 * existing lines will not be rewritten
 */
 
void appendTextToFile(String filename, String text){
  File f = new File(dataPath(filename));
  if(!f.exists()){
    createFile(f);
  }
  try {
    PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(f, true)));
    out.print(text);
    out.close();
  }catch (IOException e){
      e.printStackTrace();
  }
}

/**
 * Creates a new file including all subfolders
 */
void createFile(File f){
  File parentDir = f.getParentFile();
  try{
    parentDir.mkdirs(); 
    f.createNewFile();
  }catch(Exception e){
    e.printStackTrace();
  }
}
