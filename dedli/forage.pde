import java.util.*;
int cellSize;
int cellN = 20;
String[] lines;
int[][] cells;


PImage[][] img;//W,imgE,imgS,imgN;
PImage[] sim; 

PFont font;
void setup() {
  size (900,512);
  cellSize = 512/cellN;
  
  printArray(PFont.list());
  font = createFont("Arial", 24);
  textFont(font);
  
  img = new PImage[4][2];
  img[0][0] = loadImage("wormy1N.jpg");
  img[2][0] = loadImage("wormy1S.jpg");
  img[1][0] = loadImage("wormy1W.jpg");
  img[3][0] = loadImage("wormy1E.jpg");
  img[0][1] = loadImage("wormy2N.jpg");
  img[2][1] = loadImage("wormy2S.jpg");
  img[1][1] = loadImage("wormy2W.jpg");
  img[3][1] = loadImage("wormy2E.jpg");
  
  sim = new PImage[12];
  sim[0] = loadImage("wStill.jpg");
    sim[1] = loadImage("wR.jpg");
    sim[2] = loadImage("wL.jpg");
  
  sim[3] = loadImage("wEat.jpg");
  
  sim[4] = loadImage("wS1.jpg");
  sim[5] = loadImage("wS2.jpg");
    sim[6] = loadImage("wS3.jpg");
    sim[7] = loadImage("wS4.jpg");

  sim[8] = loadImage("wEatS1.jpg");
  sim[9] = loadImage("wEatS2.jpg");
  sim[10] = loadImage("wEatS3.jpg");
  sim[11] = loadImage("wConsume.jpg");
  
  
  lines = loadStrings("visualization.txt");
   stroke(255);

  noSmooth();

  background(255); // Fill in black in case cells don't cover all the windows
  fill(0);
   rect(512,0,20,512); 
  //noLoop();
}
 
 
 int i=0;
  int wormChoice = 0;
  
void draw() {
  if (i==lines.length){
    noLoop();
  } else {
  delay(5);
  color c=color(0);
  
  //int agent =0;  
  List<String> loc=Arrays.asList(split(lines[i],' '));

   int locX = Integer.parseInt(loc.get(0));
   int locY = Integer.parseInt(loc.get(1));
   int fac = Integer.parseInt(loc.get(2));
   Float score = Float.parseFloat(loc.get(3));
   //int mode = 1;
   //Integer.parseInt(loc[4]);
   
  // int input = -1;
/*
   if (mode == 0){
   //   input = inputN;
   for (int x=0; x<cellN; x++) {
     for (int y=0; y<cellN; y++) {
      switch (Integer.parseInt(s[y])) {
        case 0:  c = color(255,255, 255); break;
        case 1:  c = color(200, 0, 0); break;
        case 2:  c = color(150, 0, 0); break;
        case 3:  c = color(100, 0, 0); break;
        case 4:  c = color(0,0,200); break;
        //default: c = color (100,0,0); agent =1; 
       }
       fill(c);
       rect (y *cellSize, x*cellSize, cellSize, cellSize);
      }
      //line(x*cellSize + cellSize/2, y*cellSize + cellSize/2, x*cellSize + cellSize/2 +10, y*cellSize + cellSize/2+ 10);
     }
   }
    else {
 */     //input = inputD;
       for (int x=0; x<cellN; x++) {
          for (int y=0; y<cellN; y++) {
      if (loc.contains(Integer.toString(x)+","+Integer.toString(y))) 
         c = color(0,0,200); 
       else c = color(255,255, 255); 
        
        //default: c = color (100,0,0); agent =1; 
      
      fill(c);
      rect (y*cellSize, x*cellSize, cellSize, cellSize);
    }
      //line(x*cellSize + cellSize/2, y*cellSize + cellSize/2, x*cellSize + cellSize/2 +10, y*cellSize + cellSize/2+ 10);
    }
    int[][] light = new int[3][2];
   switch(fac){
     case 1: for (int i=0;i<3;i++){
               light[i][0] = locX;
               light[i][1] = (locY+cellN -i -1)%cellN;
             }
             break;
              
     case 2: for (int i=0;i<3;i++){
               light[i][1] = locY;
               light[i][0] = (locX+cellN +i +1)%cellN;
             }
             break;
              
     case 3: for (int i=0;i<3;i++){
               light[i][0] = locX;
               light[i][1] = (locY+cellN +i +1)%cellN;
             }
             break;
             
     case 0: for (int i=0;i<3;i++){
               light[i][1] = locY;
               light[i][0] = (locX+cellN -i -1)%cellN;
             }
              break;
   }
   for (int i=0;i<3;i++){
      fill(color(0,100+(70*i),0));
      rect (light[i][1]*cellSize, light[i][0]*cellSize, cellSize, cellSize);
   }         
   
    
    
   wormChoice = (wormChoice + 1)%2;
    
    image(img[fac][wormChoice],locY*cellSize, locX*cellSize,cellSize,cellSize);          
 
 /*  int simImage=-1;
  // print(input);
   switch(output){
     case 1: simImage = 2; break; // Left
     case 2: simImage = 1; break; // Right
     case 3:  // move
       switch(input){
         case 0: simImage = 0; break; //
         case 1: simImage = 4; break; //
         case 2: simImage = 5; break; //
         case 3: simImage = 6; break; //         
         case 4: simImage = 7; break; //
       }
       break;
     case 0:  // eat 
       switch(input){
         case 0: simImage =  3; break; //
         case 1: simImage = 8; break; //
         case 2: simImage = 9; break; //
         case 3: simImage = 10; break; //         
         case 4: simImage = 11; break; // eat food
       }
       break;
     }
   
   image(sim[simImage],580,200,300,300);
   */
   //if (output==0 && input==4) food++;
   fill(255);
   rect(600,0,300,200);
   textAlign(LEFT);
   fill(0);
    text("Score: " + Float.toString(score), 700, 80);
    //drawBody(input,output);
   
   
// saveFrame("results/esw2_D/agent_###.png");
  i++;  
}
  }