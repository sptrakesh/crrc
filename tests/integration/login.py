#!/usr/bin/env python3
import argparse
from ghost import Ghost, Session

parser = argparse.ArgumentParser(description="Login to CRRC application.")
parser.add_argument("--baseUrl", required=True, help="Enter base URL for CRRC application. (eg. http://localhost:3000/)" )

args = parser.parse_args()
print( "Logging into to CRRC at: {}".format( args.baseUrl ) )

ghost = Ghost()
with ghost.start() as session:
  page, _resources = session.open( args.baseUrl )
  assert page.http_status == 200 and 'Username' in page.content
  session.set_field_value( "#username", "admin" )
  session.set_field_value( "#password", "admin" )
  session.evaluate( "document.getElementById('submit').click();", expect_loading=True )
  session.wait_for_page_loaded()
  assert session.exists( "#welcome" )
#  result, _resources = session.evaluate(
#    "document.getElementById('welcome').innerHTML;" )
#  print( result )
