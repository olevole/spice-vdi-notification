_server_name='samson.bsdstore.ru';

var vdi={
	
	start:function()
	{
		this.wsconnect();
	},
	
	wsconnect:function()
	{
		this.client_id=Math.random(10000);	// поменять на сессию
		this.socket = new WebSocket("ws://"+_server_name+":9001/");	//+location.pathname
		$(this.socket).on('open',$.proxy(this.wsopen,this))
			.on('close',$.proxy(this.wsclose,this))
			.on('error',$.proxy(this.wserror,this))
			.on('message',$.proxy(this.wsmessage,this));
	},
	wsopen:function(event)
	{
		//this.notify('Соединение по вебсокету успешно открыто!','success');
		this.connected=true;
		setTimeout($.proxy(this.if_wsopened,this),500);
	},
	if_wsopened:function()
	{
		if(!this.connected) return;
		
	},
	wsclose:function(event)
	{
		if(event.wasClean)
		{
			var msg_type='warning';
			var msg='Сервер закрыл соединение!';
		}else{
			var msg_type='error';
			var msg='Соединение с сервером разорвано аварийно! Перезагрузите страницу!';
		}
		//this.notify(msg,msg_type);
		this.connected=false;
		setTimeout($.proxy(this.wsconnect,this),5000);
		//$('#net-stat').attr('class','offline icon-online');
		
		this.closeList(true);
	},
	wserror:function(error)
	{
		this.connected=false;
		//this.notify(error.message,'error');
	},
	wsmessage:function(event)
	{
		try{
			var msg=JSON.parse(event.originalEvent.data);
		}catch(e){
/*
			var msg=event.originalEvent.data;
			if(msg.substr(0,5)=='JSON:')
			{
				data=JSON.parse(msg.substr(5));
				this.onEndOperation(data);
			}

			//this.notify(msg,'warning');
			return;
*/
		}
		
		if(msg && typeof msg.command!='undefined')
		{
			if(msg.command=='update')
			{
				this.updateList(msg);
			}
			return;
		}
		
		if(msg && msg.length>0)
		{
			if(typeof msg[0]!='undefined' && msg[0]['id']!='undefined')
			{
				this.closeList();
				this.makeList(msg);
			}
		}
		
		/*
		if(msg && msg.author=='system')
		{
			var msg=JSON.parse(msg.body);
			if(this.client_id!=msg.client_id && msg.path==location.pathname)
			{
				if(msg.cmd=='update')
				{
					this.enableWait(msg.jail_id);
					this.tasks.add({'operation':msg.operation,'jail_id':msg.jail_id,'status':msg.status,'task_id':msg.task_id,'txt_status':msg.txt_status});
					this.tasks.start();
				}
				if(msg.cmd=='reload')
				{
					this.loadData('getJsonPage',$.proxy(this.onLoadData,this));
				}
			}
			return;
		}
		
		if(msg)
		{
			var txt='<storng>'+msg.author+':</storng> '+msg.body;
			this.notify(txt,'information');
		}
		*/
	},
	wssend:function(txt,user)
	{
		var author='user';
		if(typeof user!='undefined') author=user;
		if(typeof txt=='object')
		{
			txt.client_id=this.client_id;
			txt=JSON.stringify(txt);
		}
		var msg=JSON.stringify({'author':author,'body':txt});
		this.socket.send(msg);
	},
	
	
	makeList:function(data)
	{
		for(n=0,nl=data.length;n<nl;n++)
		{
			var d=data[n];
			if(typeof data[n]['group'] != 'undefined')
				var g=data[n]['group'];
			else g=1;
			if(typeof data[n]['status'] != 'undefined')
				var s=data[n]['status'];
			else s=0;
			$('#g'+g).append('<li id="m'+d.id+'"><i class="fa fa-desktop s'+s+'"></i><span class="name">'+d.name+'</span></li>');
		}
	},
	closeList:function(mode)
	{
		var error=false;
		var msg='';
		if(typeof mode=='undefined') error=false; else error=mode;
		var gs=$('ol.mon');
		if(error) msg='<li>Connection closed!</li>';
		for(n=0,nl=gs.length;n<nl;n++)
			if(error) $(gs[n]).html(msg);
	},
	updateList:function(data)
	{
		if(typeof data['id'] != 'undefined')
		{
			var id=this.dotEscape(data['id']);
			if(typeof data['status'] != 'undefined')
			{
				var cl=$('#m'+id+' i').attr('class');
				$('#m'+id+' i').attr('class',cl.replace(/s[\d+]/,'s'+data['status']));
			}
			if(typeof data['name'] != 'undefined')
				$('#m'+id+' span.name').html(data['name']);
		}
	},
	
	dotEscape:function(txt)
	{
		return txt.replace(/\./,'\\\.');
	},
}

vdi.start();