
  <footer class="main-footer">
    <div class="pull-right hidden-xs">
      <b>Version</b> 2.x
    </div>
    <strong>Copyright &copy; 2020 <a href="https://abls-habitat.fr">ABLS-Habitat</a>.</strong> All rights reserved.
  </footer>

  <!-- Control Sidebar -->
 <aside class="control-sidebar control-sidebar-dark">

	<section id="control-bar-content" ></section>

  </aside>
  <!-- /.control-sidebar -->
  <!-- Add the sidebar's background. This div must be placed
       immediately after the control sidebar -->
  <div class="control-sidebar-bg"></div>


</div>
<!-- ./wrapper -->

<!-- jQuery 3 -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/jquery/dist/jquery.min.js"></script>
<!-- jQuery UI 1.11.4 -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/jquery-ui/jquery-ui.min.js"></script>
<!-- Resolve conflict in jQuery UI tooltip with Bootstrap tooltip -->
<script>
  $.widget.bridge('uibutton', $.ui.button);
</script>
<!-- Bootstrap 3.3.7 -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/bootstrap/dist/js/bootstrap.min.js"></script>
<!-- charts.js -->
<script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.1.4/Chart.min.js"></script>
<!-- Sparkline -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/jquery-sparkline/dist/jquery.sparkline.min.js"></script>
<!-- jvectormap -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/plugins/jvectormap/jquery-jvectormap-1.2.2.min.js"></script>
<script src="<?=base_url()?>assets/frameworks/adminlte2/plugins/jvectormap/jquery-jvectormap-world-mill-en.js"></script>
<!-- jQuery Knob Chart -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/jquery-knob/dist/jquery.knob.min.js"></script>
<!-- daterangepicker -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/moment/min/moment.min.js"></script>
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/bootstrap-daterangepicker/daterangepicker.js"></script>
<!-- datepicker -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/bootstrap-datepicker/dist/js/bootstrap-datepicker.min.js"></script>
<!-- Bootstrap WYSIHTML5 -->
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.41.0/codemirror.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.41.0/mode/clike/clike.min.js"></script>
<!-- Slimscroll -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/jquery-slimscroll/jquery.slimscroll.min.js"></script>
<!-- FastClick -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/bower_components/fastclick/lib/fastclick.js"></script>
<!-- AdminLTE App -->
<script src="<?=base_url()?>assets/frameworks/adminlte2/dist/js/adminlte.min.js"></script>

<!-- DataTables -->
<script type="text/javascript" src="https://cdn.datatables.net/1.10.20/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="https://cdn.datatables.net/1.10.20/js/dataTables.bootstrap.min.js"></script>
<script type="text/javascript" src="https://cdn.datatables.net/responsive/2.2.3/js/dataTables.responsive.min.js"></script>
<script type="text/javascript" src="https://cdn.datatables.net/responsive/2.2.3/js/responsive.bootstrap.min.js"></script>

<script>
$(function () {
	//bootstrap toltip
    $("body").tooltip({ selector: '[data-toggle=tooltip]' });
});
</script>

<!-- CUSTOM JS -->
<script>var base_url = "<?=base_url()?>";</script>
<script src="<?=base_url()?>assets/js/custom/syn.js"></script>
<script src="<?=base_url()?>assets/js/custom/dls.js"></script>
<?php if (isset($load_js)) echo "<script src='".base_url()."assets/js/custom/".$load_js."'></script>"; ?>


<script>
$(".callout").fadeTo(5000, 500).slideUp(500, function(){
    $(".callout").slideUp(500);
});
$(".c-sidebar").click(function() {
  $("html, body").animate({ scrollTop: 0 }, "slow");
});

</script>


</body>
</html>
