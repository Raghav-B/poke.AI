if exist "userconfig\svnrev.h" goto :SkipCopy
copy /Y "defaultconfig\svnrev.h" "userconfig\svnrev.h"

:SkipCopy
if exist "defaultconfig\.svn" goto :OldSVN
defaultconfig\SubWCRev.exe .. ".\defaultconfig\svnrev_template.h" ".\userconfig\svnrev.h"
goto :End

:OldSVN
defaultconfig\SubWCRevOld.exe .. ".\defaultconfig\svnrev_template.h" ".\userconfig\svnrev.h"

:End
