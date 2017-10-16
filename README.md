# CRRC application
Application for managing articulation agreements between educational institutions.

## Sections
* [Technology Stack](#techStack)
* [Managing Entities](#entities)
  * [Institutions](#institutions)
  * [Contacts](#contacts)
  * [Departments](#departments)
  * [Designations](#designations)


## <a name="techStack"></a>Technology Stack
The application is built using [Cutelyst](https://cutelyst.org/) with
[Grantlee](https://github.com/steveire/grantlee) templating engine and
[Qt](https://www.qt.io/). At present only storage to [SQLite](http://sqlite.org/)
is supported, although there are plans to support [PostgreSQL](https://www.postgresql.org/)
as well. A C++14 compatible compiler is required to build the application sources.

### Development Environment
The application was developed primarily on Windows 10 with Visual Studio 2015
and CLion on Mac OS X.  Docker container testing was performed only on Mac OS X.

### Deploying locally
The easiest way to get the application running locally is to use [Docker](https://www.docker.com/).
Use the `<path to crrc>/docker/build.sh` followed by `<path to crrc>/docker/run.sh`
to build and run a docker container with the latest version of the application.

The `run.sh` script attempts to bind local port 80 to the CRRC application.  If
the binding was successful, you can visit the application [locally](http://localhost/).

For local testing, use `admin` as both username and password to log on to the
application.

## <a name="entities"></a>Managing Entities
Entities may be managed using the CRRC web UI, or using some rudimentary
web services (note, these are not true REST services).  The primary purpose of
the web services is to allow for more independent web UI development using
client side logic and rendering without depending too much upon Cutelyst
templates.

The templates for managing entities follow a common naming convention:
*  `<path to>/index.html` - Used to display a list of entities.
*  `<path to>/view.html` - Used to display details of an entity.
*  `<path to>/form.html` - Used to create/edit entities.

### <a name="institutions"></a>Institutions
Institutions are the logical starting point for managing entities within the
system.  Most information managed by the application deal with institutions.

#### Screens
The following screens and associated web services are available:

*  All templates for managing institution entities are under the `root/src/institutions/`
   directory.
*  Index or listing page.  This page is rendered by the `index.html` template.
   *   The list of institutions may also be retrieved as a JSON array by making a
     `POST` request to the page instead of a `GET` request.
*  Detail page.  This page displays the pertinent details about an institution.
   This page is rendered by the `view.html` template.
   *  A JSON representation of the institution may be retrieved by making a
      `GET` request to the `/institutions/id/<id of institution>/data` endpoint.
*  Edit page.  This page displays a form that is used to create or edit an
   institution and is rendered by the `form.html` template.
   *  The form `POST`'s the information to `/institutions/edit` endpoint.  The
      resulting created/modified entity will be displayed on screen.
   *  Using `PUT` instead of `POST` will return a JSON representation of the
      created/modified institution.
*  Check unique.  This end point returns the text `true` or `false` is the
   specified institution name and city is not unique.  Used when creating an
   institution to avoid duplicates.  The end point may be accessed at
   `/institutions/checkUnique/?name=<name value>&city=<city value>`.
*  Delete end point.  This end point is used to delete existing institutions.
   * A `POST` request to `/institutions/remove` will remove the entity and
     display the index/list page.
   * A `PUT` request to `/institutions/remove` will remove the entity and
     return a JSON message indicating success/failure of the operation.

### <a name="contacts"></a>Contacts
Contacts are the relevant staff at the institutions who manage the data in the
application.  Contacts may be assigned credentials and roles for managing the
data in the application.  Contacts may also be stored without any credentials,
in which case they serve as just useful information relating to an institution.

#### Screens
The following screens and associated web services are available:

*  All templates for managing institution entities are under the `root/src/contacts/`
   directory.
*  Index or listing page.  This page is rendered by the `index.html` template.
   *   The list of contacts may also be retrieved as a JSON array by making a
     `POST` request to the page instead of a `GET` request.
*  Detail page.  This page displays the pertinent details about a contact.
   This page is rendered by the `view.html` template.
   *  A JSON representation of the contact may be retrieved by making a
      `GET` request to the `/contacts/id/<id of contact>/data` endpoint.
*  Edit page.  This page displays a form that is used to create or edit a
   contact and is rendered by the `form.html` template.
   *  The form `POST`'s the information to `/contacts/edit` endpoint.  The
      resulting created/modified entity will be displayed on screen.
   *  Using `PUT` instead of `POST` will return a JSON representation of the
      created/modified contact.
*  Check username.  This end point is used to validate whether a specified
   `username` value is available for use or not.  Returns the text `true` or
   `false`.  The end point may be accessed at `/contacts/isUsernameAvailable/?username=<value>`.
*  Delete end point.  This end point is used to delete existing contacts.
   * A `POST` request to `/contacts/remove` will remove the entity and
     display the index/list page.
   * A `PUT` request to `/contacts/remove` will remove the entity and
     return a JSON message indicating success/failure of the operation.
     
## <a name="departments"></a>Departments
Departments are managed primarily at the institution level.  Service based
management is available through the following end points.

*  Index or listing. Listing of all the departments (if global administrator)
   or of the departments associated with institution of current user is returned
   as a JSON array by the `/departments` endpoint.
*  JSON data representation of a department may be retrieved using the
   `/departments/id/<id value>/data` endpoint.
*  New departments may be created, or existing ones updated using the
   `/institution/departments/save` endpoint.  A JSON structure indicating whether the
   insert/update succeeded is returned by the service.
*  Departments may be deleted using the `/institution/departments/remove`
   endpoint.  A JSON structure indicating success/failure of the operation is
   returned.
   
## <a name="designations"></a>Designations
At present there is no web UI to manage designations.  Designations are metadata
that are associated with institutions and programs.  Web service endpoints are
available for managing designations.

*  Index or listing. Listing of all the designations as a JSON array may be
   retrieved via the `/designations` endpoint.
*  JSON data representation of a designation may be retrieved using the
   `/designations/id/<id value>/data` endpoint.
*  New designations may be created, or existing ones updated using the
   `/designations/save` endpoint.  A JSON structure indicating whether the
   insert/update succeeded is returned by the service.
*  Designations may be deleted using the `/designations/remove`
   endpoint.  A JSON structure indicating success/failure of the operation is
   returned.

### Screens
Departments for an institution are managed using a single `index.html` template
which is available under the `root/src/institutions/departments` directory.
The various CRUD operations are supported as JSON services which are implemented
in the template.

## Tests
Integration tests are being developed for the backend interactions and are
available under the `tests/service` directory.

UI tests are being planned using Python and [Ghost](https://github.com/jeanphix/Ghost.py).
A sample login/logout test script is available under `tests/ui` directory.
