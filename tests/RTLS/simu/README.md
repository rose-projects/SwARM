# Simulation script

## Goal

This script is meant to simulate a trilateralisation with noise measurements,
and display the results. It allows to study to influence of computing the average
of 2 and 4 measurements on precision.

## Algorithm

* a point is chosen randomly
* distances between beacons and point are computed
* gaussian noise is added to the measurements
* trilateralisation is performed
* error between the real and measurement points is computed

## How to run this script

Open `index.html` with your favourite browser.

It displays :
* 1000 measurement errors in mm in red
* the average of 2 successive measurements in pink
* the average of 4 successive measurements in yellow
