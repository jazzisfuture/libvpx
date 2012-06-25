#!/bin/bash
#
# hybrid transform coding scheme experimental configuration

./vpxenc -o /tmp/test_enc.ivf /export/hda3/Videos/bus_cif.y4m --good --cpu-used=0 -t 0 -w 352 -h 288 --fps=30000/1000 --lag-in-frames=0 --target-bitrate=2000 --min-q=0 --max-q=63 --end-usage=0 --codec=vp8 --auto-alt-ref=1 -p 2 --pass=1 --fpf=/tmp/tmp_vp8_testht.fpf --kf-max-dist=9999 --kf-min-dist=0 --drop-frame=0 --static-thresh=0 --yv12 --bias-pct=50 --minsection-pct=0 --maxsection-pct=800 --arnr-maxframes=7 --arnr-strength=6 --arnr-type=3 --sharpness=0 --psnr --limit=40  2>&1 | tr '\r' '\n'

./vpxenc -o /tmp/test_enc.ivf /export/hda3/Videos/bus_cif.y4m --good --cpu-used=0 -t 0 -w 352 -h 288 --fps=30000/1000 --lag-in-frames=0 --target-bitrate=2000 --min-q=0 --max-q=63 --end-usage=0 --codec=vp8 --auto-alt-ref=1 -p 2 --pass=2 --fpf=/tmp/tmp_vp8_testht.fpf --kf-max-dist=9999 --kf-min-dist=0 --drop-frame=0 --static-thresh=0 --yv12 --bias-pct=50 --minsection-pct=0 --maxsection-pct=800 --arnr-maxframes=7 --arnr-strength=6 --arnr-type=3 --sharpness=0 --psnr --limit=40 --test-decode  2>&1 | tr '\r' '\n' 
