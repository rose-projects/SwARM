$ = require 'jquery'
path = require '../lib/path'

playing = no
date = 0
duration = 100

$ ->
	player = document.getElementById('player')

	$('.play-slider').change ->
		date = $(this).val()
		player.currentTime = date/10
		$('.simu-date').text(date)

	player.ontimeupdate = ->
		date = Math.round(player.currentTime*10)
		$('.play-slider').val(date)
		$('.simu-date').text(date)
		$('.play-slider').attr(max: player.duration*10) if duration != player.duration

	player.onended = ->
		playing = no
		$('.play-btn').removeClass('fa-pause').addClass('fa-play')
		player.pause()
		# end simu



	$('.play-btn').click ->
		if playing
			$(this).addClass('fa-play').removeClass('fa-pause')
			player.pause()
		else
			$(this).addClass('fa-pause').removeClass('fa-play')
			player.play()
		playing = not playing

	$('.stop-btn').click ->
		playing = no
		$('.play-btn').removeClass('fa-pause').addClass('fa-play')
		player.pause()
		date = 0
		$('.play-slider').val(date)
		$('.simu-date').text(date)
		player.currentTime = 0
