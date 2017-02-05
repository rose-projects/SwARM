# Choregraph

A GUI to design and execute dances.

* create and visualise moves
* set color changes of the dances
* simulate the creation synchronised with the music
* download the dance to the robots with the click of a button
* one click dance start/stop with music playback
* easily send commands to the master beacon for calibration/diagnostics

## Install

To install Choregraph you need :
* node (and npm)
* coffescript (`sudo npm install -g coffee-script`)

Then cd to the choregraph directory and run `make install`

*Note:* the install has been tested on few configuration and may need support.

## Usage

To run choregraph, the easiest way is to run `make run`

*Note :* to test simulation (the upper toolbar with player control), you need to
add your own music in ressources, in file named "audio.mp3"

## Architecture

src/ : the sources of the app in coffeescript

view/ : jade templates

ressources/fonts/ : fonts used in the app

ressources/images : images used in the app

ressources/scripts/ : javascript libraries made for browser

ressources/styles/ : CSS stylesheets for the project's display

ressources/index.jade : UI DOM tree (compiled into index.html)
