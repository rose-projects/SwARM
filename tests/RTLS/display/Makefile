all:
	coffee -o lib/ -c src/

watch:
	coffee -o lib/ -cw src/

clean:
	rm -rf lib/

run: all
	electron .

dist:
	rm -rf dist/
	electron-packager ./ --platform=darwin,linux --arch=x64 --out=./dist/

.PHONY: all watch clean test run dist
