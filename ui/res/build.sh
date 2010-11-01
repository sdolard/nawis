#!/bin/sh
[ ! -f famfamfam_silk_icons_v013.zip ] && wget http://www.famfamfam.com/lab/icons/silk/famfamfam_silk_icons_v013.zip
[ ! -d famfamfam_silk_icons_v013 ] && unzip famfamfam_silk_icons_v013.zip -d famfamfam_silk_icons_v013
[ ! -f index_abc.png ] && wget http://www.famfamfam.com/lab/icons/silk/previews/index_abc.png
