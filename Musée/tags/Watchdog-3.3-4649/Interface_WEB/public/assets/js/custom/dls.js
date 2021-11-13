$(document).ready(function() {

		$("#dls").DataTable( {
			"pageLength" : 100,
			"ajax": {
				url : base_url + "admin/dls/get",
				type : 'GET'
			},
   "order": [ [5, "asc"], [6, "asc"] ],
			responsive: true,
		});

  var myCodeMirror = CodeMirror.fromTextArea( document.getElementById("sourcecode"), { lineNumbers: true } );
  myCodeMirror.setSize( null, "100%");

	});
