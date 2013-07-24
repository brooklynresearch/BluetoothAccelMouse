/*
 Bluetooth Accelerometer Mouse
 
 Reads an Analog Devices ADXL335 accelerometer.  The acceleration readings are smoothed
 and adjusted to be sent as mouse X and Y values.  The z axis of the accelerometer is used to
 indicate a left mouse click.  The readings are then sent to the roving networks HID bluetooth 
 module RN-42.

 The Circuit:
 Leonardo:
 analog 2:   x-axis - Accelerometer
 analog 3:   y-axis - Accelerometer
 analog 4:   z-axis - Accelerometer
 digital 0:  TXO - RN-42
 digital 1:  RXI - RN-42
 
 created 23 Jul 2013
 by Ezer Longinus
 using libraries from 
 BPLib by Basel
 and some code from ladyada
 
 This example code is in the public domain.

*/

#include <BPLib.h>

//------------------------------------------//
// PARAMETERS TO CHANGE FOR PERSONAL USAGE  //
//------------------------------------------//

// The paramater below adjust the threshold of when the mouse should
// start to move.  
// If the mouse is too shakey and it's hard to stabalize
// increase the number.  If the movements need to be too exagerated until the mouse
// move decrease the number
const int cintMovementThreshold = 10;


// The parameters below marks what the default/mouse does not move position.
// If your arm is at rest an the mouse starts to drift in the x or y position
// adjust accordingly.
// ex: If mouse drifts left when arm is at rest decrease cintZeroXValue.
const int cintZeroXValue = 345;
const int cintZeroYValue = 350;

// The parameters below mark what the maximum (positive) acceleration values are.
// If you arm is full extended right or forward and the mouse moves to fast or slow
// adjust accordingly.
// ex: If your mouse moves too slowly fully extended to the right decrease cintMaxXValue.
// ex: If your mouse moves too fast when slight moving your arm right increase cintMaxValue.
const int cintMaxXValue = 365;
const int cintMaxYValue = 370;

// The parameters below mark what the minimum (negative) acceleration values are.
// If you arm is full extended left or behind you and the mouse moves to fast or slow
// adjust accordingly.
// ex: If your mouse moves too slowly fully extended to the left decrease cintMaxXValue.
// ex: If your mouse moves too fast when slight moving your arm left increase cintMaxValue.
const int cintMinXValue = 335;
const int cintMinYValue = 340;

// The maximum speed in each axis (x and y)
// that the cursor should move. Set this to a higher or lower
// number if the cursor does not move fast enough or is too fast.
const int cintMaxMouseMovement = 10;

// The parameter below marks the threshold reading of the accelerometer to define
// what a mouse click is.
// If the false clicks are occuring increase the value.  If too much effort is needed
// to generate a click decrease value.
const int cintZButtonThreshold = 300;

//-------------------------------------------------//
// END OF PARAMETERS TO CHANGE FOR PERSONAL USAGE  //
//-------------------------------------------------//



// Bluetooth library to send mouse data to RN-42
BPLib BPMod;


// Inputs for the Accelerometer
const int pinAnalogXInput = 2;
const int pinAnalogYInput = 3;
const int pinAnalogZInput = 4;
const int pinAnalogDummyInput = 0;



//The sign of the mouse movement relative to the acceleration.
//If your cursor is going in the opposite direction you think it
//should go, change the sign for the appropriate axis.
const int cintXSign = 1;
const int cintYSign = -1;
const int cintZSign = 1;

//const float cfloatMovementMultiplier = 1;



//This reduces the 'twitchiness' of the cursor by calling
//a delay function at the end of the main loop.
//There is a better way to do this without delaying the whole
//microcontroller, but that is left for another time or person.
const int cintMouseDelay = 8;

// A switch statement is used so that we don't constantly click the mouse
// when the the z parameter is reached.
boolean mouseClicked = false;

void setup(){
  Serial.begin(115200);
  BPMod.begin(BP_MODE_HID,BP_HID_MOUSE);

  //This is not needed and set to default but can be useful if you
  //want to get the full range out of the analog channels when
  //reading from the 3.3V ADXL335.
  //If the analog reference is used, the thresholds, zeroes,
  //maxima and minima will need to be re-evaluated.
  analogReference( DEFAULT );

}

void loop(){

  //Process the accelerometer to make the cursor move.
  fcnProcessAccelerometer();

  //Uncomment these lines to debug the accelerometer values:
  //  Serial.print("X:\t");
  //  Serial.print(analogRead(pinAnalogXInput));
  //  Serial.print("\tY:\t");
  //  Serial.print(analogRead(pinAnalogYInput));
  //  Serial.print("\tZ:\t");
  //  Serial.println(analogRead(pinAnalogZInput));

}

void fcnProcessAccelerometer()
{
  //Initialize values for the mouse cursor movement.
  int intMouseXMovement = 0;
  int intMouseYMovement = 0;

  //Read the dummy analog channel
  //This must be done first because the X analog channel was first
  //and was unstable, it dropped or pegged periodically regardless
  //of pin or source.
  analogRead( pinAnalogDummyInput );

  //Read accelerometer readings  
  int intAnalogXReading = analogRead(pinAnalogXInput);
  int intAnalogYReading = analogRead(pinAnalogYInput);
  int intAnalogZReading = analogRead(pinAnalogZInput);
  
  // Check to see if a click occured
  if(intAnalogZReading >= cintZButtonThreshold){
    mouseClicked = true;
  }

  //Calculate mouse movement
  //If the analog X reading is ouside of the zero threshold...
  if( cintMovementThreshold < abs( intAnalogXReading - cintZeroXValue ) )
  {
    //...calculate X mouse movement based on how far the X acceleration is from its zero value.
    intMouseXMovement = cintXSign * ( ( ( (float)( 2 * cintMaxMouseMovement ) / ( cintMaxXValue - cintMinXValue ) ) * ( intAnalogXReading - cintMinXValue ) ) - cintMaxMouseMovement );
    //it could use some improvement, like making it trigonometric.
  }
  else
  {
    //Within the zero threshold, the cursor does not move in the X.
    intMouseXMovement = 0;
  }

  //If the analog Y reading is ouside of the zero threshold... 
  if( cintMovementThreshold < abs( intAnalogYReading - cintZeroYValue ) )
  {
    //...calculate Y mouse movement based on how far the Y acceleration is from its zero value.
    intMouseYMovement = cintYSign * ( ( ( (float)( 2 * cintMaxMouseMovement ) / ( cintMaxYValue - cintMinYValue ) ) * ( intAnalogYReading - cintMinYValue ) ) - cintMaxMouseMovement );
    //it could use some improvement, like making it trigonometric.
  }
  else
  {
    //Within the zero threshold, the cursor does not move in the Y.
    intMouseYMovement = 0;
  }
  
  // If mouse click occured send mouse click signal to bluetooth.
  if(mouseClicked){
    BPMod.mouseClick(BP_MOUSE_BTN_LEFT);
    mouseClicked = false;
  }
  
  // Send mouse X and Y values to bluetooth
  BPMod.mouseMove(intMouseXMovement,intMouseYMovement);
  
  
  // delay is here to allow for the computer time to parse the bluetooth data.
  // If reaction time seems too slow decrease value.
  delay(100);

}

