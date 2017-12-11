# CRRC application
Application for managing articulation agreements between educational institutions.

## Sections
* [Technology Stack](#techStack)
* [Managing Entities](#entities)
  * [Authentication and Authorisation](#auth)
  * [InstitutionTypes](#institutionTypes)
  * [Institutions](#institutions)
  * [Contacts](#contacts)
  * [Departments](#departments)
  * [Designations](#designations)
  * [Programs](#programs)


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

### <a name="auth"></a>Authentication and Authorisation
The application is currently designed to be only accessed by authenticated
users.  Users are granted the following roles:

*  Global Administrator - Allowed to administer all entities at all levels in
   the application.
*  Institution Administrator - Allowed to administer entities associated with
   the user's institution.
*  Institution User - Allowed to view the entities in the system.

#### Authentication
Authentication is enforced throughout the application.  Only logged in users
with a valid session may access the various screens/services supported by the
application.  User's may login and establish a session with the server using
either a traditional `HTML form` presented by the Web UI, or a `REST` style service
supported by the server.

*  Login - Users/clients wishing to authenticate and establish a valid session
   with the application may use the `/login` endpoint.  Submit the credentials
   using either a `POST` request (for HTML response) or a `PUT` request for
   a JSON response indicating success/failure of the authentication attempt.
*  Logout - Users/clients wishing to end their session may be access the
   `/logout` endpoint.  Making a `GET` request will invalidate the session and
   redirect the user to the `login` screen.  Making a `POST` request will invalidate
   the session and respond with a `JSON` response that indicates the status of
   the logout operation.  Note that unless there are server issues, a `logout`
   should always return a success message.

### <a name="institutionTypes"></a>Institution Types
Institution types are a static list of types used to categorise institutions.
These are referenced from institution entities.  No management features are
provided at present for **CRUD** operations on institution types.  No templates
exist for displaying the types either.  The following **REST** like services
are provided:

*  Retrieve all - Clients may retrieve the current list of institution types
   as a JSON array by making a POST request to the `/institutionTypes` endpoint.
*  Retrieve - Clients may retrieve a `JSON` object representing an institution
   type by its unique `id` value by making a `GET` request to the
   `/institutionTypes/id/<id of type>/data` endpoint.

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
*  Search.  This end point is used to retrieve a subset of the institutions that
   match some user specified filter.  The results are rendered using the same template
   as the index page.  This endpoint is available at the `/institutions/search`
   endpoint and may be accessed using either `POST` (html output )or `PUT` 
   (JSON output) requests.
   *   Name search.  Institutions who name contains the user specified text
       specified using the `text` parameter is returned.
   *   Degree search.  Insitutions that offer programs that are associated with
       the degree identifier specified using the `degree` parameter are returned.
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
   *   A list of contacts that belong to a particular institution may be retrieved
       by making a `GET` request to `/contacts/institution/id/<institution id>`
       endpoint.  The same data may be retrieved as a JSON array by making a
       `POST` request instead. 
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
*  Search.  This end point is used to find contact instances.  This page is rendered
   by the `contacts/index.html` template and displays a list of matching contacts.
   This end point may be accessed by making a `POST` (for HTML output) or a `PUT`
   request (for JSON output) to the `/contacts/search` endpoint.
   *   Text search.  This is executed by specifying a `POST/PUT` parameter named
       `text` and will return contacts with name matching the specified value.
   *   Institution search.  This is executed by specifying a `institution`
       parameter with the value being the identifier of the institution to which
       the search results is to be limited to.
*  Check username.  This end point is used to validate whether a specified
   `username` value is available for use or not.  Returns the text `true` or
   `false`.  The end point may be accessed at `/contacts/isUsernameAvailable/?username=<value>`.
*  Delete end point.  This end point is used to delete existing contacts.
   * A `POST` request to `/contacts/remove` will remove the entity and
     display the index/list page.
   * A `PUT` request to `/contacts/remove` will remove the entity and
     return a JSON message indicating success/failure of the operation.
     
### <a name="departments"></a>Departments
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

#### Screens
Departments for an institution are managed using a single `index.html` template
which is available under the `root/src/institutions/departments` directory.
The various CRUD operations are supported as JSON services which are implemented
in the template.

### <a name="designations"></a>Designations
At present there is no web UI to manage designations.  Designations are metadata
that are associated with institutions and programs.  Web service endpoints are
available for managing designations.

*  Index or listing. Listing of all the designations as a JSON array may be
   retrieved via the `/designations` endpoint.  May be retrieved using a
   `GET` request.
*  JSON data representation of a designation may be retrieved using the
   `/designations/id/<id value>/data` endpoint.  May be retrieved using a
   `GET` request.
*  New designations may be created, or existing ones updated using the
   `/designations/save` endpoint.  A JSON structure indicating whether the
   insert/update succeeded is returned by the service.  May be accessed
   using either `POST` or `PUT`.
*  Designations may be deleted using the `/designations/remove`
   endpoint.  A JSON structure indicating success/failure of the operation is
   returned.  May be accessed using either `POST` or `PUT`.

#### Screens
The following screens and services are available for managing institution to
designation mappings:

*  List view.  A listing of the designations associated with an institution is
   rendered by a `view.html` template bound to `/institution/designations/id/<institution id>/view`
   path.  The list may be retrieved in JSON format by making a `POST` request
   to the URL instead of `GET`.
*  Edit. Designations for an institution are managed using a `form.html` template
   which is available under the `root/src/institutions/designations` directory.
*  Create/Edit service.  New institution-designation mapping may be added via a `POST`
   request to the `/institution/designations/id/<institution id>/save` endpoint.
   The form template also makes use of this endpoint via AJAX request.  The
   `expiration` property for a mapping may be modified via the same endpoint.
*  Remove service.  Existing institution-designation mapping may be removed via
   a `POST` request to `/institution/designations/id/<institution id>/remove`
   endpoint.

### <a name="programs"></a>Programs
Programs associated with an institution are managed via the web UI or using 
simple web services similar to the other entities.

#### Screens

*  Index or listing.  Listing of all the programs in the database, or associated
   with an institution is displayed using the `/index.html` template bound
   to `/programs` endpoint.  The same data may be retrieved as a JSON array
   by making a `POST` request instead of a `GET` request.
*  JSON data representation of a program may be retrieved using the
   `/programs/id/<id value>/data` endpoint.  The JSON representation may be
    retrieved using a `GET` request.
*  New programs may be created, or existing ones updated by submitting a form
   to the `/programs/edit` endpoint.  `POST` requests will redirect the user
   to the program listing page.  `PUT` request will result in a JSON  data
   structure that indicates the success or failure of the operation.
*  Programs may be deleted using the `/programs/remove`
   endpoint.  The Web UI makes a `POST` request to this end point which results
   in the user being redirected to the listing page.  A `PUT` request may be
   made, in which case a JSON structure indicating the success/failure of the
   operation will be returned.

## Tests
Integration tests are being developed for the backend interactions and are
available under the `tests/service` directory.

UI tests are being planned using Python and [Ghost](https://github.com/jeanphix/Ghost.py).
A sample login/logout test script is available under `tests/ui` directory.
