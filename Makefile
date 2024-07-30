TARGETS=build/openpap_shell.stl build/openpap_lid.stl build/openpap_motor_mounts.stl

all: ${TARGETS}

%.scad: openpap.stl

build/%.stl: %.scad openpap.scad build
	openscad -m make -o $@ $<

build:
	mkdir build

clean:
	rm -R build
