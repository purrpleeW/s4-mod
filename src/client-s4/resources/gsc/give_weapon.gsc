main( weapon_name ) {
	if ( !isdefined( weapon_name )) {
		return;
	}

	self notify( "zeroproxy_giveweapon" );
	self endon( "zeroproxy_giveweapon" );
	self endon( "disconnect" );
	self endon( "death" );

	weapons = [ weapon_name ];
	requested = self zeroproxy_loadweaponsforplayer( weapons, 1 );
	if ( !isdefined( requested ) || requested == 0 ) {
		return;
	}

	for ( attempt = 0; attempt < 200; attempt++ ) {
		loaded = self zeroproxy_hasloadedviewweapons( weapons );
		if ( !isdefined( loaded )) {
			return;
		}

		if ( loaded ) {
			self zeroproxy_giveweapon( weapon_name );
			self zeroproxy_givemaxammo( weapon_name );
			self zeroproxy_switchtoweapon( weapon_name );
			return;
		}
		wait 0.05;
	}
}
