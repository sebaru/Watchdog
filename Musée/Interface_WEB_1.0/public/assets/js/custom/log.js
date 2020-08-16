$(document).ready(function() {
			
		$("#log").DataTable( {
			"pageLength" : 100,
			"ajax": {
				url : base_url + "admin/log/get",
				type : 'GET'
			},
   "order": [ [0, "desc"] ],
			responsive: true,
			
		});

	});
