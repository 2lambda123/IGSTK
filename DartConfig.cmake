# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "0:30:00 EDT")

# Dart server to submit results (used by client)
IF(DROP_METHOD MATCHES http)
  SET (DROP_SITE "public.kitware.com")
  SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
ELSE(DROP_METHOD MATCHES http)
  SET (DROP_SITE "public.kitware.com")
  SET (DROP_LOCATION "/incoming")
  SET (DROP_SITE_USER "ftpuser")
  SET (DROP_SITE_PASSWORD "public")
ENDIF(DROP_METHOD MATCHES http)

SET (TRIGGER_SITE 
  "http://${DROP_SITE}/cgi-bin/Submit-IGSTK-TestingResults.pl")

# Project Home Page
#SET (PROJECT_URL "http://${DROP_SITE}/IFSTK")

# Dart server configuration 
SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/IGSTK-rollup-dashboard.sh")
SET (CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/cvsweb.cgi/IGSTK/")
SET (CVS_WEB_CVSROOT "IGSTK")

SET (USE_DOXYGEN "On")
#SET (DOXYGEN_URL "http://${DROP_SITE}/IGSTK/index.html" )

SET (USE_GNATS "On")
SET (GNATS_WEB_URL "http://public.kitware.com/Bug/index.php")

# Continuous email delivery variables
SET (CONTINUOUS_FROM "luis.ibanez@kitware.com")
SET (SMTP_MAILHOST "public.kitware.com")
SET (CONTINUOUS_MONITOR_LIST "luis.ibanez@kitware.com")
SET (CONTINUOUS_BASE_URL "http://public.kitware.com/IGSTK")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES ON)
SET (DELIVER_BROKEN_BUILD_EMAIL "Continuous Nightly")
SET (EMAIL_FROM "cmake-dashboard@public.kitware.com")
SET (DARTBOARD_BASE_URL "http://public.kitware.com/IGSTK")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_CONFIGURE_FAILURES 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_ERRORS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_WARNINGS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_NOT_RUNS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES 1)

