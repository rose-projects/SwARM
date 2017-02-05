// Generated by CoffeeScript 1.10.0
(function() {
  var addNoise, beacons, distance, error, errors, errorsMean2, errorsMean4, generateRobot, i, j, len, noise, randn, simulateError, sum, triangulate;

  beacons = {
    mb: {
      x: 0,
      y: 0
    },
    sb1: {
      x: 5000,
      y: 0
    },
    sb2: {
      x: 0,
      y: 5000
    }
  };

  noise = 20;

  distance = function(point1, point2) {
    return Math.sqrt(Math.pow(point1.x - point2.x, 2) + Math.pow(point1.y - point2.y, 2));
  };

  randn = function() {
    var u, v;
    u = 1 - Math.random();
    v = 1 - Math.random();
    return Math.sqrt(-2.0 * Math.log(u)) * Math.cos(2.0 * Math.PI * v);
  };

  generateRobot = function() {
    var result, robot;
    robot = {
      x: Math.random() * beacons.sb1.x,
      y: Math.random() * beacons.sb2.y
    };
    result = {
      mbDist: distance(robot, beacons.mb),
      sb1Dist: distance(robot, beacons.sb1),
      sb2Dist: distance(robot, beacons.sb2),
      point: robot
    };
    return result;
  };

  addNoise = function(robot) {
    var result;
    result = {
      mbDist: robot.mbDist + randn() * noise,
      sb1Dist: robot.sb1Dist + randn() * noise,
      sb2Dist: robot.sb2Dist + randn() * noise,
      point: robot.point
    };
    return result;
  };

  triangulate = function(robot) {
    var result;
    result = {
      x: ((Math.pow(robot.mbDist, 2)) - (Math.pow(robot.sb1Dist, 2)) + (Math.pow(beacons.sb1.x, 2))) / (2 * beacons.sb1.x),
      y: ((Math.pow(robot.mbDist, 2)) - (Math.pow(robot.sb2Dist, 2)) + (Math.pow(beacons.sb2.y, 2))) / (2 * beacons.sb2.y)
    };
    return result;
  };

  simulateError = function() {
    var robot;
    robot = generateRobot();
    return distance(robot.point, triangulate(addNoise(robot)));
  };

  errors = (function() {
    var j, results;
    results = [];
    for (i = j = 1; j <= 1000; i = ++j) {
      results.push(simulateError());
    }
    return results;
  })();

  errorsMean2 = (function() {
    var j, ref, results;
    results = [];
    for (i = j = 1, ref = errors.length; 1 <= ref ? j <= ref : j >= ref; i = 1 <= ref ? ++j : --j) {
      results.push((errors[i] + errors[i - 1]) / 2);
    }
    return results;
  })();

  errorsMean4 = (function() {
    var j, ref, results;
    results = [];
    for (i = j = 3, ref = errors.length; 3 <= ref ? j <= ref : j >= ref; i = 3 <= ref ? ++j : --j) {
      results.push((errors[i] + errors[i - 1] + errors[i - 2] + errors[i - 3]) / 4);
    }
    return results;
  })();

  sum = 0;

  for (j = 0, len = errors.length; j < len; j++) {
    error = errors[j];
    sum += error;
  }

  console.log("max: " + (Math.max.apply(Math, errors)) + ", mean: " + (sum / errors.length));

  errors.sort(function(a, b) {
    return a - b;
  });

  errorsMean2.sort(function(a, b) {
    return a - b;
  });

  errorsMean4.sort(function(a, b) {
    return a - b;
  });

  new Chartist.Line('.ct-chart', {
    series: [errors, errorsMean2, errorsMean4]
  });

}).call(this);
