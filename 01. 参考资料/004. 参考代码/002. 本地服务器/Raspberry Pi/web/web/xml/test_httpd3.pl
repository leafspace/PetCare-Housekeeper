use strict;
use warnings;

use FindBin;
use lib $FindBin::Bin;

use HTTP::Daemon;
use HTTP::Status;
use threads;
use Win32::IPConfig;
use Data::Dump qw(dump);
use Digest::MD5;
use Digest::SHA::PurePerl qw(sha256_base64 hmac_sha256_hex hmac_sha256_base64);
use List::Util qw(min); 
use MIME::Base64;
use threads;
use IO::FIle;

&main();
################################################################
# main
#
sub main{
	$|=1;
	
	my $d = MyHttpd->new({
		ports		=> [80, 10080]
	});
	
	printf "### httpd server start!\n";
	while (1) {
		$d->recv_request ();

		my $body;
		if ($d->{method} eq "GET") {
			$body = read_file ($d->{uri});
		} elsif ($d->{method} eq "POST") {
			write_file ($d->{uri}, $d->{body});
			$body = read_file ($d->{uri});
		}
		
		$d->send_response ({status => 200, body => $body});
	fin:
		$d->{socket}->close ();
	}
}


################################################################
sub read_file {
	my $file = shift;
	
	$file =~ s|^/|./|;

	my $io = IO::File->new();
	$io->open($file, "r");
	
	my $line;
	my $body = "";
	while ($line = <$io>) {
		$body .= $line;
	}
	$io->close;
	
	return $body;
	
}

################################################################
sub write_file {
	my $file = shift;
	my $body = shift;
	
	$file =~ s|^/|./|;
	$file =~ s|/([^/]*)$|/Posted-$1|;
	
	printf  ("write file at $file\n");
	
	my $io = IO::File->new();
	$io->open($file, "w");
	$io->write ($body);
	$io->close;
	printf  ("b\n");
	return;
}

################################################################
package MyHttpd;
use strict;
use IO::Socket;
use Digest::MD5 qw(md5_hex);
use List::Util qw(min); 
use IO::Select;

sub new {
	my $pkg = shift;
	my $param = shift;
	my $self = $param;
	
	$self->{select} = IO::Select->new();
	
	foreach my $port (@{$param->{ports}}) {
		my $listen = IO::Socket::INET->new(LocalPort => $port,
										   Proto    => 'tcp',
										   Listen    => 5);
		
		binmode ($listen);
		$self->{select}->add ($listen);
	}
	
	bless $self, $pkg;
}


sub recv_request {
	my $self = shift;
	
	printf  "### wait msg...\n";
	
	my ($accept) = $self->{select}->can_read();
	
	my $socket = $accept->accept();
#	my $socket = $self->{listensocket}->accept ();
	$self->{socket} = $socket;
	printf  "### ------ recv response ------\n";
	
	# request line
	my $req_line = <$socket>;
	print "$req_line";
	
	my @tmp = split / /, $req_line;
	
	$self->{method} = $tmp[0];
	$self->{uri} = $tmp[1];
	$self->{ver} = $tmp[2];
	
	# request header
	my %auth_info;
	my $content_len = 0;
	undef $self->{auth_info};
	my %header;
	while ( <$socket> ){
		print "$_";
		
		last if $_ eq "\r\n";
		
		
		$_ =~ s/[\r\n]+$//g;
		
		my ($name, $val) = split ( /:/ );
		
		$name =~ s/^ +//g;
		$name =~ s/ +$//g;
		$val =~ s/^ +//g;
		$val =~ s/ +$//g;
		
		$header{$name} = $val;
	}
	$self->{header} = \%header;
	$content_len = int $header{"Content-Length"} if defined $header{"Content-Length"};
	
	# read body
	$self->{body} = "";
			
	while ($content_len != 0) {
		my $data;
		$socket->read ($data, min(1024, $content_len));
		next if (length($data) == 0); # fin received
		$content_len -= length $data;
		$self->{body} .= $data;
	}
	print "$self->{body}\n\n";
	
	
}

sub send_response {
	my $self = shift;
	my $param = shift;
	
	printf  "### ------ send response ------\n";
	printf  "### status = $param->{status}\n";
	
	my $len = 0;
	$len = length ($param->{body}) if defined $param->{body};
	
	my $connection = "close";
	$connection = $param->{connection} if defined $param->{connection};
	
	my $user_header = "";
	$user_header = $param->{user_header} if defined $param->{user_header};
	
	if ($param->{status} == 200) {
		$self->{socket}->send ("HTTP/1.1 200 Ok\r\n");
		$self->{socket}->send ("Content-Length: $len\r\n");
		$self->{socket}->send ("Connection: close\r\n");
		$self->{socket}->send ("$user_header");
		$self->{socket}->send ("\r\n");
		
	} elsif ($param->{status} == 301) {
		$self->{socket}->send ("HTTP/1.1 301 Ok\r\n");
		$self->{socket}->send ("Content-Length: $len\r\n");
		$self->{socket}->send ("Connection: close\r\n");
		$self->{socket}->send ("Location: $param->{location}\r\n");
		$self->{socket}->send ("\r\n");
		
	} elsif ($param->{status} == 401) {
		$self->{socket}->send ("HTTP/1.1 401 Authorization Required\r\n");
		$self->{socket}->send ("Content-Length: $len\r\n");
		$self->{socket}->send ("Connection: close\r\n");
		$self->{socket}->send ("WWW-Authenticate: " . $param->{auth} );
		$self->{socket}->send ("\r\n");
		$self->{socket}->send ("\r\n");
		
		printf "$param->{auth}\n";
		
	} else {
		$self->{socket}->send ("HTTP/1.1 500 Internal Server Error\r\n");
		$self->{socket}->send ("Content-Length: $len\r\n");
		$self->{socket}->send ("Connection: close\r\n");
		$self->{socket}->send ("\r\n");
	}
	$self->{socket}->send ($param->{body}) if defined $param->{body};
	printf "$param->{body}\n\n";
	
	
}
