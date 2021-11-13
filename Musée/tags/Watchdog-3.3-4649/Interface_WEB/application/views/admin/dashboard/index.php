<?php
defined('BASEPATH') OR exit('No direct script access allowed');

?>

            <div class="content-wrapper">
                <section class="content-header">
                    <?php echo $pagetitle; ?>
                    <?php echo $breadcrumb; ?>
                </section>

                <section class="content">

                    <label>Elements graphiques</label>
                    <div class="row">
                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-maroon"><i class="fa fa-image"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Synoptiques</span>
                                    <span class="info-box-number"><?php echo $count_syns; ?></span>
                                </div>
                            </div>
                        </div>
                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-maroon"><i class="fa fa-image"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Motifs</span>
                                    <span class="info-box-number"><?php echo $count_syns_motifs; ?></span>
                                </div>
                            </div>
                        </div>
                    </div>
                    <label>D.L.S</label>
                    <div class="row">
						                  <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-aqua"><i class="fa fa-cogs"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Modules D.L.S</span>
                                    <span class="info-box-number"><?php echo $count_dls; ?></span>
                                </div>
                            </div>
                        </div>

                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-aqua"><i class="fa fa-code"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Lignes D.L.S</span>
                                    <span class="info-box-number"><?php echo $nbr_lignes_dls; ?></span>
                                </div>
                            </div>
                        </div>

                        <div class="clearfix visible-sm-block"></div>

                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-blue"><i class="fa fa-wrench"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Mnemoniques</span>
                                    <span class="info-box-number"><?php echo $count_mnemos; ?></span>
                                </div>
                            </div>
                        </div>

						                  <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-green"><i class="fa fa-envelope"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Messages</span>
                                    <span class="info-box-number"><?php echo $count_msgs; ?></span>
                                </div>
                            </div>
                        </div>
                    </div>
                    <label>Système</label>
                    <div class="row">
                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-brown"><i class="fa fa-user"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Utilisateurs</span>
                                    <span class="info-box-number"><?php echo $count_users; ?></span>
                                </div>
                            </div>
                        </div>

                        <div class="col-md-3 col-sm-6 col-xs-12">
                            <div class="info-box">
                                <span class="info-box-icon bg-black"><i class="fa fa-database"></i></span>
                                <div class="info-box-content">
                                    <span class="info-box-text">Logs</span>
                                    <span class="info-box-number"><?php echo $count_logs; ?></span>
                                </div>
                            </div>
                        </div>


                    </div>

                  <div class="row">
                    <div class="col-md-6 col-sm-12 col-xs-12">
                						<div class="box box-primary">
						                  <div class="box-header ui-sortable-handle" style="cursor: move;">
						                    <i class="fa fa-clipboard"></i><h3 class="box-title">Service status</h3>
                								</div>

                								<!-- /.box-header -->
								                <div class="box-body">
                								  <ul class="todo-list ui-sortable">
									                   <li>
                    										<span class="text"><b>Version : </b><?=$status->version?></span><br>
										                    <span class="text"><b>Instance : </b><?=$status->instance?></span><br>
										                    <span class="text"><b>Is master : </b><?=$status->instance_is_master?></span><br>
										                    <span class="text"><b>Run_As : </b><?=$status->run_as?></span><br>
									  	                  <span class="text"><b>Started : </b><?=$status->started?></span><br>
										                    <span class="text"><b>License : </b><?=$status->license?></span><br>
									  	                  <span class="text"><b>Author Name : </b><?=$status->author_name?></span><br>
										                    <span class="text"><b>Author Email : </b><?=$status->author_email?></span><br>
										                    <span class="text"><b>Top : </b><?=$status->top?></span><br>
									  	                  <span class="text"><b>Bit par Sec  : </b><?=$status->bit_par_sec?></span><br>
										                    <span class="text"><b>Tour par Sec  : </b><?=$status->tour_par_sec?></span><br>
									  	                  <span class="text"><b>Lenght i : </b><?=$status->length_i?></span><br>
										                    <span class="text"><b>Lenght msg : </b><?=$status->length_msg?></span><br>
									         							 		</li>
									      								   </ul>
                        </div>
             							  </div>
                    </div>
                    <div class="col-md-6 col-sm-12 col-xs-12">
                						<div class="box box-primary">
						                  <div class="box-header ui-sortable-handle" style="cursor: move;">
						                    <i class="fa fa-database"></i><h3 class="box-title">Database Configuration</h3>
                								</div>

                								<!-- /.box-header -->
								                <div class="box-body">
                								  <ul class="todo-list ui-sortable">
									                   <li>
                    										<span class="text"><b>DB Username : </b><?php echo $db_username ?></span><br>
                    										<span class="text"><b>DB Hostname : </b><?php echo $db_hostname ?></span><br>
                    										<span class="text"><b>DB Database : </b><?php echo $db_database ?></span><br>
                    										<span class="text"><b>ArchDB Username : </b><?php echo $archdb_username ?></span><br>
                    										<span class="text"><b>ArchDB Hostname : </b><?php echo $archdb_hostname ?></span><br>
                    										<span class="text"><b>ArchDB Database : </b><?php echo $archdb_database ?></span><br>
									         							 		</li>
									      								   </ul>
                        </div>
             							  </div>
                    </div>
                  </div>


                    <label>Monitoring</label>

                    <div class="row">
                      <div class="col-md-12">
                        <div class="col-md-6 col-sm-12 col-xs-12">
                        <canvas id="dls_tour_per_sec"> </canvas>
                        </div>

                        <div class="col-md-6 col-sm-12 col-xs-12">
                           <canvas id="dls_bit_per_sec"> </canvas>
                        </div>
                      </div>
             					  </div>

                    <div class="row">
                      <div class="col-md-12">
                        <div class="col-md-6 col-sm-12 col-xs-12">
                           <canvas id="dls_wait"> </canvas>
                        </div>
						                  <div class="col-md-6 col-sm-12 col-xs-12">
                           <canvas id="arch_request_number"> </canvas>
               							  </div>

                      </div>
             					  </div>

                </section>
            </div>
