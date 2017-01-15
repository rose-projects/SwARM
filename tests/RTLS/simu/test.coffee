beacons =
	mb:
		x: 0
		y: 0
	sb1:
		x: 5000
		y: 0
	sb2:
		x: 0
		y: 5000

noise = 20

distance = (point1, point2) -> Math.sqrt((point1.x-point2.x)**2 + (point1.y-point2.y)**2)
randn = ->
    u = 1 - Math.random()
    v = 1 - Math.random()
    return Math.sqrt( -2.0 * Math.log( u ) ) * Math.cos( 2.0 * Math.PI * v )

generateRobot = ->
	robot =
		x: Math.random()*beacons.sb1.x
		y: Math.random()*beacons.sb2.y
	result =
		mbDist: distance robot, beacons.mb
		sb1Dist: distance robot, beacons.sb1
		sb2Dist: distance robot, beacons.sb2
		point: robot
	return result

addNoise = (robot) ->
	result =
		mbDist: robot.mbDist + randn()*noise
		sb1Dist: robot.sb1Dist + randn()*noise
		sb2Dist: robot.sb2Dist + randn()*noise
		point: robot.point
	return result

triangulate = (robot) ->
	result =
		x: ((robot.mbDist**2) - (robot.sb1Dist**2) + (beacons.sb1.x**2))/(2*beacons.sb1.x);
		y: ((robot.mbDist**2) - (robot.sb2Dist**2) + (beacons.sb2.y**2))/(2*beacons.sb2.y);
	return result

simulateError = ->
	robot = generateRobot()
	return distance robot.point, triangulate addNoise robot

errors = (simulateError() for i in [1..1000])
errorsMean2 = ((errors[i]+errors[i-1])/2 for i in [1..errors.length])
errorsMean4 = ((errors[i]+errors[i-1]+errors[i-2]+errors[i-3])/4 for i in [3..errors.length])
sum = 0
sum += error for error in errors

console.log "max: #{Math.max(errors...)}, mean: #{sum/errors.length}"
errors.sort((a, b)-> a-b)
errorsMean2.sort((a, b)-> a-b)
errorsMean4.sort((a, b)-> a-b)

new Chartist.Line('.ct-chart', series: [errors, errorsMean2, errorsMean4]);
