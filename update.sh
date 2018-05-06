#!/bin/sh

"../../output/bin/bmfs" --disk "../../output/baremetal-os.img" --offset 32KiB rm -f "Applications/httpd.app"
"../../output/bin/bmfs" --disk "../../output/baremetal-os.img" --offset 32KiB cp "httpd.app" "Applications/httpd.app"
