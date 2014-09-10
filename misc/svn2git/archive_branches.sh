#!/bin/bash

branches=(
	"VendorARL"
	"ansi-branch"
	"attr-extension-mods"
	"autoconf-branch"
	"bobWinPort"
	"bottie"
	"brlcad_5_1_alpha_patch"
	"cmake"
	"dmtogl"
	"goblin"
	"libpng"
	"nurbs"
	"phong-branch"
	"photonmap-branch"
	"rel-5-1-branch"
	"rel-6-0-1-branch"
	"rel-7-0-branch"
	"rel-7-4-branch"
	"rel-7-6-branch"
	"scriptics"
	"trimnurbs-branch"
	"unlabeled-1.1.1"
	"unlabeled-1.1.2"
	"unlabeled-1.2.1"
	"unlabeled-11.1.1"
	"unlabeled-2.12.1"
	"unlabeled-2.5.1"
	"unlabeled-2.6.1"
	"unlabeled-9.1.1"
	"unlabeled-9.10.1"
	"unlabeled-9.12.1"
	"unlabeled-9.2.1"
	"unlabeled-9.3.1"
	"unlabeled-9.7.1"
	"unlabeled-9.9.1"
	"windows-branch"
	"zlib"
	)

for i in "${branches[@]}"
do
	echo "Archiving $i"
	git tag archive/$i $i
	git branch -D $i
done

