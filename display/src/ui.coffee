$ = require 'jquery'
ipc = require('electron').ipcRenderer
app = require('electron').remote.app

robots = []

intRead = (cl, def) ->
	val = parseInt $(".#{cl}").val()
	return if isNaN(val) then def else val
sb1x = -> intRead('sb1x', 100)
sb2y = -> intRead('sb2y', 100)

point2serie = (point) ->
	serie  = (null for i in [1..point[0]])
	serie.push point[1]
	return serie
updateChart = ->
	labels = [0..sb1x()]
	labels[i] = null for i in labels when i%10 != 0

	series = [[0], [sb2y()], point2serie([sb1x(), 0])]
	series.push point2serie(rb) for rb in robots

	new Chartist.Line('.ct-chart', {labels: labels, series: series})

ipc.on 'robots', (event, points) ->
	robots = points
	updateChart()
ipc.on 'msg', (event, msg) ->
	$('.log').append msg
ipc.on 'seriallist', (event, list) ->
	for port in list
		$('.serialpath').append "<option value=\"#{port.comName}\">#{port.comName}</option>"

$ ->
	$('.beacons-btn').click ->
		updateChart()
		ipc.send 'beacons', sb1x(), sb2y()
	$('.serial-btn').click ->
		$('.log').text('')
		ipc.send 'connect', $('.serialpath').val(), intRead('baudrate', 38400)
	$('.calib-btn').click ->
		ipc.send 'calib', intRead('calib-id', 1), intRead('calib-x', 50), intRead('calib-y', 50), [sb1x(), 0], [0, sb2y()]

	updateChart()
	ipc.send 'ready'
