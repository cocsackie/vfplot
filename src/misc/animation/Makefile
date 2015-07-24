# Makefile for vfplot animation

VFPLOT = ../../vfplot/vfplot
RUBBISH = anim.*.*.eps anim.*.*.png cylinder.mp4 cylinder.eps

cylinder.mp4 : eps png mp4

eps :
	$(VFPLOT) -v \
	  --test cylinder \
	  --iterations 200/5 \
	  --ke-drop 15 \
	  --animate \
	  --pen 0 \
	  --domain-pen 2 \
	  --ellipse \
	  --ellipse-pen 0.5m \
	  --scale 0.15 \
	  --width 5i \
	  --output cylinder.eps

png :
	ls -1 anim.*.*.eps | xargs -P4 -n1 sh mkpng.sh

mp4 :
	ffmpeg -y \
	  -r 25 \
	  -pattern_type glob -i 'anim.*.*.png' \
	  "-c:v" libx264 \
	  -pix_fmt yuv420p \
	cylinder.mp4

clean :
	$(RM) $(RUBBISH)

.PHONY : eps png mp4 clean
