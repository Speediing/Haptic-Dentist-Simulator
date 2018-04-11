# Introduction

The purpose of this project is to simulate brushing teeth. This is done as a way to train hygien- ists with different dental tools without the unnecessary pain a patient would go through when under the brush of a new hygienist. Currently only one tool exists, the basic toothbrush. The project uses Chai3d, a C++ library for haptic rendering and the Novint Falcon. This simulation occurs in a 3D world where forces are applied when the hygienist moves the toothbrush onto the teeth or gums.

![alt-text](https://s3-us-west-1.amazonaws.com/www.jasonwiker.ca/assets/img/haptics2.png){: .size-right} 

# Methods

The simulation works through the use of 3D objects being rendered into the world. Each of these objects is given their own texture and bump map. These textures are then altered as the hygienist applies pressure to the teeth. The more pressure applied, the faster the plaque is removed from the teeth. Additionally, the teeth become smoother; a clean tooth has lower friction and no bumps whereas a dirty tooth has a higher friction with more randomly distributed bumps.
The toothbrush contains 8 haptic points. This was done through the extension of the cGeneric-Tool class. These haptic points are used to generate force when they come into contact with an object. The collision detection is done using the Chai3d
framework. After each point’s force is obtained, they are all averaged. The average force value is then returned to the user through the device.
The simulation has been gamified by adding a timer and a scoring system. The more plaque that is removed from the teeth, the higher the score. However, if the hygienist touches the gums their score is lowered.

# Results

It was found that our simulator works for a tool with multiple haptic points. However, the collision detection and force algorithms runtime is large enough that adding more
than 8 haptic points causes the haptic frames to fall below 800 Hz. This causes runtime errors to occur. As such, we limited the number of points
to 8, which means the entire brush does not contain haptic points.
The texture map we applied helps us to haptically discover where the plaque is and where has been cleaned. We found this adds a layer of interactivity that you would not get from a purely graphical simulation. In addition, as the teeth get cleaned, you can feel varying levels of depth of the texture so you know what amount has been cleaned. The bump map slowly becomes scaled down depending on the force applied to the area being cleaned.
