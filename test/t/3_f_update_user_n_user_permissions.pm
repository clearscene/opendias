package t::3_f_update_user_n_user_permissions;

use lib qw( lib );
use DBI;
use standardTests;
use Data::Dumper;
use HTTP::Cookies;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
  }; 
} 

sub test {

  my $admin_user_cookie_jar = HTTP::Cookies->new();
  login( "admin", "admin", $admin_user_cookie_jar );

  my $normal_user_cookie_jar = HTTP::Cookies->new();
  login( "test-user", "password", $normal_user_cookie_jar );

  # Someone else User details update cbacking
  ###########################################

  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As a normal user, Attempt to update someone elses user
  o_log( "As a normal user, Attempt to update someone elses user" );
  o_log( Dumper( directRequest( { __cookiejar => $normal_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => 'admin',
                                  password => 'text admin',
                                  realname => 'text admin',
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As an admin user, Attempt to update someone elses user
  o_log( "As an admin user, Attempt to update someone elses user" );
  o_log( Dumper( directRequest( { __cookiejar => $admin_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => 'test-user',
                                  realname => 'text new',
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As an admin user, Attempt to update someone elses role
  o_log( "As an admin user, Attempt to update someone elses role" );
  o_log( Dumper( directRequest( { __cookiejar => $admin_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => 'test-user',
                                  role => 5,
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # Own user details update cbacking
  ##################################

  # As a normal user, Attempt to update your own user
  o_log( "As a normal user, Attempt to update your own user" );
  o_log( Dumper( directRequest( { __cookiejar => $normal_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => '[current]',
                                  password => 'text',
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As an admin user, Attempt to update your own user
  o_log( "As an admin user, Attempt to update your own user" );
  o_log( Dumper( directRequest( { __cookiejar => $admin_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => '[current]',
                                  password => 'text',
                                  realname => 'text',
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As a normal user, Attempt to update your own role
  o_log( "As a normal user, Attempt to update your own role" );
  o_log( Dumper( directRequest( { __cookiejar => $normal_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => 'test-user',
                                  role => 3,
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );

  # As an admin user, Attempt to update your own role
  o_log( "As an admin user, Attempt to update your own role" );
  o_log( Dumper( directRequest( { __cookiejar => $admin_user_cookie_jar, 
                                  action => 'updateUser', 
                                  username => 'admin',
                                  role => 3,
                                } ) ) );
  dumpQueryResult( "SELECT * FROM user_access ORDER BY username" );


  return 0;
}

return 1;

