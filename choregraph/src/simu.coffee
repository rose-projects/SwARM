$ = require 'jquery'

module.exports = (paths) ->
	playing = no
	date = 0
	duration = 100

	$ ->
		player = document.getElementById('player')

		$('.play-slider').mousedown ->
			playing = no
			return true
		$('.play-slider').mouseup ->
			playing = yes

		$('.play-slider').change ->
			date = $(this).val()
			player.currentTime = date/10
			$('.simu-date').text(date)
			path.showSimAt(date) for path in paths

		timer = ->
			if playing
				date = Math.round(player.currentTime*10)
				$('.play-slider').val(date)
				$('.simu-date').text(date)
				$('.play-slider').attr(max: player.duration*10) if duration != player.duration
				path.showSimAt(date) for path in paths
		setInterval(timer, 50)

		player.onended = ->
			playing = no
			$('.play-btn').removeClass('fa-pause').addClass('fa-play')
			player.pause()

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
			path.hideSim() for path in paths
