#!/usr/bin/env python3
import argparse
from ghost import Ghost, Session

parser = argparse.ArgumentParser(description="Login to CRRC application.")
parser.add_argument("--baseUrl", required=True, help="Enter base URL for CRRC application. (eg. http://localhost:3000/)" )
parser.add_argument("--user", required=False, default="admin", help="Enter username to use to login to CRRC application." )
parser.add_argument("--password", required=False, default="admin", help="Enter password to use to login to CRRC application." )

args = parser.parse_args()
print( "Logging into to CRRC at: {} as user: {}".format( args.baseUrl, args.user ) )

ghost = Ghost()
with ghost.start() as session:
  page, _resources = session.open( args.baseUrl )
  assert page.http_status == 200 and 'Username' in page.content
  session.set_field_value( "#username", args.user )
  session.set_field_value( "#password", args.password )
  session.evaluate( "document.getElementById('submit').click();", expect_loading=True )
  session.wait_for_page_loaded()
  assert session.exists( "#welcome" )
#  result, _resources = session.evaluate(
#    "document.getElementById('welcome').innerHTML;" )
#  print( result )
