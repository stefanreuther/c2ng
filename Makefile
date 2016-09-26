#
# Makefile
#

OBJECTS_c2export = tools/c2export.o
LDFLAGS = $(CONFIG_AFL_LDFLAGS) $(CONFIG_C2NG_LDFLAGS)
OBJECTS_c2ng = main/c2ng.o
OBJECTS_gamelib = game/pcc/accountfolder1.o game/pcc/gamefolder1.o \
    game/pcc/serverdirectory.o game/pcc/browserhandler1.o \
    game/interface/globalcommands.o game/interface/simpleprocedure.o \
    game/historyturnlist.o game/historyturn.o util/backupfile.o \
    game/map/objectlist.o game/map/objectreference.o game/nu/turnloader.o \
    game/nu/specificationloader.o game/nu/gamestate.o \
    game/nu/registrationkey.o game/nu/stringverifier.o game/nu/gamefolder.o \
    game/nu/accountfolder.o game/interface/minefieldfunction.o \
    game/interface/minefieldcontext.o game/interface/minefieldproperty.o \
    game/map/minefieldtype.o game/map/minefield.o \
    game/interface/playerfunction.o game/interface/playercontext.o \
    game/interface/torpedofunction.o game/interface/torpedocontext.o \
    game/spec/torpedo.o game/interface/hullfunction.o \
    game/interface/hullcontext.o game/interface/hullproperty.o \
    game/interface/beamfunction.o game/interface/beamcontext.o \
    game/interface/enginefunction.o game/interface/enginecontext.o \
    game/interface/engineproperty.o game/interface/weaponproperty.o \
    game/interface/componentproperty.o interpreter/exporter/htmlexporter.o \
    game/interface/userinterfacepropertystack.o game/extracontainer.o \
    game/map/renderer1.o game/map/viewport.o game/map/renderoptions.o \
    game/map/renderlist.o game/teamsettings.o \
    game/interface/ionstormfunction.o game/interface/ionstormcontext.o \
    game/interface/ionstormproperty.o game/tables/ionstormclassname.o \
    game/tables/wormholestabilityname.o \
    game/tables/mineraldensityclassname.o game/tables/headingname.o \
    game/map/ionstormtype.o game/map/ionstorm.o \
    game/interface/globalfunctions.o game/interface/objectcommand.o \
    game/interface/iteratorcontext.o game/map/objectobserver1.o \
    game/interface/richtextfunctions.o game/interface/richtextvalue.o \
    game/interface/simplefunction.o game/stringverifier1.o \
    game/v3/stringverifier2.o game/map/cursors.o \
    game/map/simpleobjectcursor.o game/map/objectcursor.o util/keystring.o \
    game/spec/basecomponentvector.o game/interface/shipfunction.o \
    game/interface/shipcontext.o game/interface/shipproperty.o \
    game/browser/handlerlist.o game/msg/inbox.o game/exception.o \
    game/browser/session1.o game/browser/usercallbackproxy.o \
    util/rich/parser1.o game/nu/browserhandler.o \
    game/browser/unsupportedaccountfolder.o util/rich/linkattribute.o \
    util/rich/styleattribute.o util/skincolor.o util/rich/colorattribute.o \
    util/rich/visitor.o util/rich/text.o util/profiledirectory.o \
    game/browser/accountmanager.o game/browser/account.o \
    util/requestthread.o game/browser/directoryhandler.o \
    game/browser/filesystemrootfolder.o game/browser/rootfolder.o \
    game/browser/filesystemfolder.o game/browser/browser.o \
    game/score/compoundscore.o game/score/turnscorelist.o \
    game/score/turnscore.o game/interface/playerproperty.o \
    interpreter/exporter/jsonexporter.o interpreter/exporter/textexporter.o \
    interpreter/exporter/dbfexporter.o util/constantanswerprovider.o \
    interpreter/exporter/separatedtextexporter.o \
    interpreter/exporter/exporter.o interpreter/exporter/fieldlist.o \
    interpreter/processlist.o game/interface/globalcontext.o \
    game/interface/globalproperty.o game/interface/planetfunction.o \
    game/interface/planetcontext.o interpreter/objectpropertyvector.o \
    game/tables/basemissionname.o game/interface/baseproperty.o \
    game/map/historyshiptype.o game/map/playedshiptype.o \
    game/map/anyshiptype.o game/tables/happinesschangename.o \
    game/map/planetformula.o game/map/shiphistorydata.o game/map/shipdata.o \
    game/map/ship.o game/spec/missionlist.o game/spec/mission.o \
    game/shipbuildorder.o game/tables/mineralmassclassname.o \
    game/tables/happinessname.o game/tables/temperaturename.o \
    game/tables/industrylevel.o game/tables/nativegovernmentname.o \
    game/tables/nativeracename.o interpreter/closure.o \
    interpreter/hashvalue.o interpreter/hashdata.o \
    game/interface/planetproperty.o interpreter/savevisitor.o \
    game/session2.o game/game.o game/unitscoredefinitionlist.o \
    game/unitscorelist.o interpreter/selectionexpression.o \
    interpreter/filecommandsource.o interpreter/filevalue.o \
    interpreter/subroutinevalue.o interpreter/memorycommandsource.o \
    interpreter/binaryexecution.o interpreter/indexablevalue.o \
    interpreter/arguments.o interpreter/arrayvalue.o interpreter/process.o \
    interpreter/structurevalue.o interpreter/structuretype.o \
    interpreter/basevalue.o interpreter/ternaryexecution.o \
    interpreter/keymapvalue.o interpreter/unaryexecution.o \
    util/keymaptable.o util/keymap.o util/atomtable.o util/key.o \
    interpreter/singlecontext.o interpreter/propertyacceptor.o \
    interpreter/nametable.o interpreter/simplespecialcommand.o \
    interpreter/defaultstatementcompilationcontext.o interpreter/keywords.o \
    interpreter/statementcompiler.o interpreter/commandsource.o \
    interpreter/statementcompilationcontext.o interpreter/context1.o \
    game/parser/messageparser.o game/parser/messagetemplate.o \
    game/map/rangeset.o game/v3/inboxfile.o game/parser/messageinformation.o \
    game/parser/messagevalue.o game/turnloader1.o game/v3/loader.o \
    game/playerbitmatrix.o game/v3/resultloader.o game/turn.o \
    interpreter/optimizer.o interpreter/fusion.o interpreter/world.o \
    interpreter/values.o interpreter/expr/parser.o \
    interpreter/expr/builtinfunction.o interpreter/ternaryoperation.o \
    interpreter/unaryoperation.o interpreter/binaryoperation.o \
    interpreter/expr/indirectcallnode.o interpreter/expr/functioncallnode.o \
    interpreter/expr/identifiernode.o interpreter/expr/membernode.o \
    interpreter/expr/logicalnode.o interpreter/expr/conditionalnode.o \
    interpreter/expr/sequencenode.o interpreter/expr/assignmentnode.o \
    interpreter/expr/casenode.o interpreter/expr/simplenode.o \
    interpreter/expr/simplervaluenode.o interpreter/expr/literalnode.o \
    interpreter/expr/rvaluenode.o interpreter/expr/node.o \
    interpreter/bytecodeobject.o interpreter/opcode.o util/math.o \
    interpreter/error.o interpreter/tokenizer.o \
    game/config/userconfiguration.o game/map/anyplanettype.o \
    game/map/playedbasetype.o game/map/playedplanettype.o \
    game/map/objecttype.o game/map/universe.o game/map/configuration1.o \
    game/map/basestorage.o game/map/planet.o game/map/circularobject.o \
    game/map/mapobject.o game/map/point.o game/map/object.o \
    game/spec/friendlycodelist.o game/spec/friendlycode.o \
    game/vcr/classic/database.o game/vcr/statistic.o game/vcr/score.o \
    game/vcr/classic/pvcralgorithm.o game/vcr/classic/statustoken.o \
    game/vcr/classic/hostalgorithm.o game/vcr/classic/nullvisualizer.o \
    game/vcr/classic/algorithm.o game/vcr/classic/battle.o \
    game/spec/standardcomponentnameprovider.o game/vcr/object1.o \
    util/consolelogger.o game/v3/rootloader.o game/v3/directoryscanner.o \
    util/randomnumbergenerator.o game/v3/turnfile.o game/timestamp.o \
    game/spec/nullcomponentnameprovider.o game/config/configurationparser.o \
    util/configurationfileparser.o game/playerlist.o game/player.o \
    game/root.o game/spec/hullassignmentlist.o game/v3/registrationkey1.o \
    game/hostversion.o game/spec/hullfunctionassignmentlist.o \
    game/spec/hull.o game/config/aliasoption.o game/config/costarrayoption.o \
    game/config/hostconfiguration.o game/config/enumvalueparser.o \
    game/config/bitsetvalueparser.o game/config/genericintegerarrayoption.o \
    game/config/stringoption.o game/config/integeroption.o \
    game/config/configuration.o game/config/booleanvalueparser.o \
    game/config/integervalueparser.o game/config/valueparser.o \
    game/config/configurationoption.o game/spec/modifiedhullfunctionlist.o \
    game/spec/hullfunctionlist.o util/string.o game/spec/hullfunction1.o \
    game/playerset.o game/experiencelevelset.o \
    game/spec/basichullfunctionlist.o game/spec/basichullfunction.o \
    util/fileparser.o game/v3/resultfile.o game/cargospec.o \
    game/v3/specificationloader1.o game/spec/shiplist.o game/spec/engine.o \
    game/spec/torpedolauncher.o game/spec/beam.o game/spec/weapon.o \
    game/spec/component.o game/spec/cost.o game/element.o
OBJECTS_guilib = client/tiles/shipoverviewtile.o \
    client/dialogs/turnlistdialog.o client/widgets/turnlistbox.o \
    client/tiles/errortile.o client/tiles/tilefactory.o \
    ui/widgets/focusiterator.o client/screens/playerscreen.o \
    client/screens/controlscreen.o client/map/widget.o client/map/proxy.o \
    client/map/renderer.o client/dialogs/consoledialog.o \
    client/si/outputstate.o client/si/inputstate.o \
    client/dialogs/objectselectiondialog.o gfx/nullengine.o \
    client/tiles/selectionheadertile.o client/session.o \
    client/objectobserverproxy.o client/objectobserver.o client/marker.o \
    client/si/commands.o client/widgets/keymapwidget.o client/si/control.o \
    client/si/userside.o client/si/scriptside.o client/si/scriptprocedure.o \
    ui/dialogs/messagebox.o ui/rich/statictext.o \
    client/widgets/busyindicator.o ui/widgets/tiledpanel.o \
    ui/widgets/keydispatcher.o ui/invisiblewidget.o client/usercallback.o \
    client/screens/browserscreen.o ui/widgets/richlistbox.o \
    ui/rich/imageobject.o ui/rich/documentview.o ui/rich/document.o \
    ui/widgets/statictext1.o ui/widgets/inputline.o \
    ui/widgets/simpleiconbox.o ui/widgets/iconbox.o ui/group.o \
    client/widgets/folderlistbox.o ui/eventloop.o ui/skincolorscheme.o \
    gfx/clipfilter.o ui/scrollablewidget.o ui/widgets/abstractlistbox.o \
    ui/res/provider.o ui/res/directoryprovider.o ui/res/manager.o gfx/blit.o \
    ui/res/ccimageloader.o ui/res/engineimageloader.o gfx/timerqueue.o \
    gfx/rgbapixmap.o gfx/palettizedpixmap.o gfx/types.o gfx/canvas.o \
    gfx/point1.o ui/widgets/button.o ui/widgets/abstractbutton.o ui/window.o \
    ui/draw.o ui/defaultresourceprovider.o gfx/fontlist.o gfx/fontrequest.o \
    ui/spacer.o ui/layoutablegroup.o ui/layout/vbox.o ui/layout/axislayout.o \
    ui/layout/hbox.o ui/colorscheme1.o gfx/complex.o \
    gfx/sdl/streaminterface.o gfx/multiclipfilter.o gfx/filter.o ui/root1.o \
    ui/simplewidget.o ui/layout/info.o ui/cardgroup.o ui/widget1.o \
    gfx/bitmapfont.o gfx/bitmapglyph.o gfx/font.o gfx/nullcolorscheme.o \
    gfx/context.o gfx/colorscheme.o gfx/graphicsexception.o \
    gfx/sdl/engine1.o gfx/sdl/surface.o gfx/nullcanvas.o gfx/rectangleset.o \
    gfx/rectangle.o gfx/fillpattern.o
OBJECTS_testsuite = u/t_game_hostversion.o u/t_game_experiencelevelset.o \
    u/t_game_spec_engine.o u/t_game_historyturnlist.o u/t_game_element.o \
    u/t_game_historyturn.o u/t_game_turnloader.o u/t_game_registrationkey.o \
    u/t_game_specificationloader.o u/t_game_timestamp.o \
    u/t_util_backupfile.o u/t_interpreter_exporter_fieldlist.o \
    u/t_game_map_objectlist.o u/t_game_map_objectreference.o \
    u/t_ui_widgets_focusiterator.o u/t_ui_colorscheme.o u/t_ui_widget.o \
    u/t_client_dialogs_objectselectiondialog.o u/t_game_game.o \
    u/t_game_interface_userinterfacepropertystack.o u/t_util_skincolor.o \
    u/t_util_math.o u/t_util_constantanswerprovider.o \
    u/t_util_answerprovider.o u/t_game_extraidentifier.o \
    u/t_game_extracontainer.o u/t_game_map_viewport.o \
    u/t_game_map_circularobject.o u/t_game_map_mapobject.o \
    u/t_game_map_object.o u/t_game_tables_basemissionname.o \
    u/t_game_teamsettings.o u/t_game_tables_ionstormclassname.o \
    u/t_game_tables_wormholestabilityname.o \
    u/t_game_tables_mineraldensityclassname.o u/t_game_tables_headingname.o \
    u/t_game_interface_iteratorprovider.o u/t_client_objectcursorfactory.o \
    u/t_client_si_control.o u/t_gfx_eventconsumer.o u/t_gfx_nullengine.o \
    u/t_client_si_contextprovider.o u/t_ui_res_imageloader.o \
    u/t_ui_res_manager.o u/t_client_objectlistener.o \
    u/t_interpreter_keywords.o u/t_game_vcr_score.o u/t_game_vcr_object.o \
    u/t_game_parser_datainterface.o u/t_game_interface_richtextfunctions.o \
    u/t_game_v3_stringverifier.o u/t_game_stringverifier.o \
    u/t_client_si_usertask.o u/t_client_si_requestlink2.o \
    u/t_client_si_requestlink1.o u/t_game_cargospec.o \
    u/t_game_interface_userinterfacepropertyaccessor.o \
    u/t_game_interface_userinterfaceproperty.o u/t_game_map_objectcursor.o \
    u/t_game_types.o u/t_game_interpreterinterface.o u/t_game_vcr_battle.o \
    u/t_game_vcr_database.o u/t_game_vcr_classic_statustoken.o \
    u/t_game_vcr_classic_algorithm.o u/t_game_vcr_classic_nullvisualizer.o \
    u/t_game_vcr_classic_visualizer.o u/t_game_spec_componentnameprovider.o \
    u/t_game_spec_componentvector.o u/t_game_spec_component.o \
    u/t_util_keystring.o u/t_interpreter_process.o \
    u/t_interpreter_processlist.o u/t_interpreter_bytecodeobject.o \
    u/t_util_slaverequestsender.o u/t_util_slaverequest.o \
    u/t_util_baseslaverequestsender.o u/t_util_baseslaverequest.o \
    u/t_util_slaveobject.o u/t_game_browser_handlerlist.o u/t_game_player.o \
    u/t_game_config_configurationoption.o u/t_game_config_costarrayoption.o \
    u/t_game_msg_mailbox.o u/t_game_browser_unsupportedaccountfolder.o \
    u/t_ui_invisiblewidget.o u/t_game_exception.o u/t_gfx_resourceprovider.o \
    u/t_ui_rich_imageobject.o u/t_util_rich_parser.o \
    u/t_ui_rich_blockobject.o u/t_gfx_canvas.o u/t_game_browser_account.o \
    u/t_util_unicodechars.o u/t_util_rich_visitor.o \
    u/t_util_rich_attribute.o u/t_util_rich_styleattribute.o \
    u/t_util_rich_linkattribute.o u/t_util_rich_colorattribute.o \
    u/t_util_rich_text.o u/t_gfx_engine.o u/t_util_requestthread.o \
    u/t_ui_group.o u/t_game_browser_handler.o u/t_game_browser_folder.o \
    u/t_ui_res_provider.o u/t_gfx_timerqueue.o u/t_gfx_timer.o \
    u/t_gfx_sdl_engine.o u/t_util_requestreceiver.o u/t_util_request.o \
    u/t_util_requestdispatcher.o u/t_gfx_nullcanvas.o \
    u/t_interpreter_nametable.o u/t_game_tables_happinesschangename.o \
    u/t_game_spec_missionlist.o u/t_game_tables_mineralmassclassname.o \
    u/t_game_tables_happinessname.o u/t_game_tables_temperaturename.o \
    u/t_game_tables_nativeracename.o u/t_game_tables_nativegovernmentname.o \
    u/t_game_tables_industrylevel.o u/t_interpreter_closure.o \
    u/t_interpreter_values.o u/t_game_unitscoredefinitionlist.o \
    u/t_game_unitscorelist.o u/t_interpreter_statementcompiler.o \
    u/t_interpreter_expr_builtinfunction.o u/t_interpreter_expr_parser.o \
    u/t_interpreter.o u/t_util_keymaptable.o u/t_util_keymap.o \
    u/t_util_atomtable.o u/t_util_key.o u/t_game_parser_messagetemplate.o \
    u/t_game_playerbitmatrix.o u/t_game_playerarray.o \
    u/t_interpreter_tokenizer.o u/t_game_spec_friendlycodelist.o \
    u/t_game_spec_friendlycode.o u/t_game_map_configuration.o \
    u/t_game_map_point.o u/t_game_vcr_classic_pvcralgorithm.o \
    u/t_game_vcr_classic_hostalgorithm.o u/t_gfx_rectangle.o u/t_gfx_point.o \
    u/t_gfx_fillpattern.o u/t_util_randomnumbergenerator.o u/t_util_string.o \
    u/t_game_config_valueparser.o u/t_game_config_hostconfiguration.o \
    u/t_game_config_bitsetvalueparser.o u/t_game_config_booleanvalueparser.o \
    u/t_game_config_integervalueparser.o u/t_game_config_configuration.o \
    u/t_game_spec_hullfunctionlist.o u/t_game_v3_resultfile.o testsuite.o \
    u/t_game_spec_cost.o
OBJECTS_afl = 
CXXFLAGS = -I$(CONFIG_C2NG_AFL_DIR) -I. $(CONFIG_C2NG_CXXFLAGS) -MMD -g
HEADERS_testsuite = u/t_*.hpp
CXXTESTDIR = $(CONFIG_AFL_CXXTESTDIR)

include config.mk
include $(CONFIG_C2NG_AFL_DIR)/config.mk

.PHONY: tags clean distclean afl gamelib guilib

.SUFFIXES: .cpp .lo .o .s

all: all-sdl-$(CONFIG_C2NG_HAVE_SDL)

all-sdl-yes: all-sdl-no c2ng testsuite

all-sdl-no: c2export

c2export: $(OBJECTS_c2export) libgamelib.a $(LIBDEPEND)
	@echo "        Linking c2export..."
	@$(CXX) $(LDFLAGS) -o c2export $(OBJECTS_c2export) -L. -lgamelib -L$(CONFIG_C2NG_AFL_DIR) -lafl $(CONFIG_AFL_LIBS) $(LIBS)

c2ng: $(OBJECTS_c2ng) libguilib.a libgamelib.a $(LIBDEPEND)
	@echo "        Linking c2ng..."
	@$(CXX) $(LDFLAGS) -o c2ng $(OBJECTS_c2ng) -L. -lguilib -L. -lgamelib -L$(CONFIG_C2NG_AFL_DIR) -lafl $(CONFIG_AFL_LIBS) $(LIBS) $(CONFIG_C2NG_GUILIBS)

libgamelib.a: $(OBJECTS_gamelib)
	@echo "        Archiving libgamelib.a..."
	@$(AR) cru libgamelib.a $(OBJECTS_gamelib) 

libguilib.a: $(OBJECTS_guilib)
	@echo "        Archiving libguilib.a..."
	@$(AR) cru libguilib.a $(OBJECTS_guilib) 

testsuite: $(OBJECTS_testsuite) libguilib.a libgamelib.a $(LIBDEPEND)
	@echo "        Linking testsuite..."
	@$(CXX) $(LDFLAGS) -o testsuite $(OBJECTS_testsuite) -L. -lguilib -L. -lgamelib -L$(CONFIG_C2NG_AFL_DIR) -lafl $(CONFIG_AFL_LIBS) $(LIBS) $(CONFIG_C2NG_GUILIBS)

Makefile: P9/Settings P9/Rules-unix.mak P9/Settings-unix.mak
	@echo "        Regenerating Makefile..."
	@proj9 update

clean:
	$(RM) $(OBJECTS_gamelib)
	$(RM) $(OBJECTS_guilib)
	$(RM) $(OBJECTS_c2export)
	$(RM) $(OBJECTS_c2ng)
	$(RM) $(OBJECTS_testsuite)

depend.mk: Makefile
	@echo "        Regenerating depend.mk..."
	@for i in $(OBJECTS_afl) $(OBJECTS_gamelib) $(OBJECTS_guilib) $(OBJECTS_c2export) $(OBJECTS_c2ng) $(OBJECTS_testsuite); do echo "-include $${i%o}d"; done > depend.mk

distclean: clean
	$(RM) testsuite.cpp
	$(RM) libgamelib.a
	$(RM) libguilib.a
	$(RM) c2export
	$(RM) c2ng
	$(RM) testsuite

game/browser/session1.lo: game/browser/session.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/browser/session1.lo -c game/browser/session.cpp

game/browser/session1.o: game/browser/session.cpp
	@echo "        Compiling game/browser/session.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/browser/session1.o -c game/browser/session.cpp

game/browser/session1.s: game/browser/session.cpp
	$(CXX) $(CXXFLAGS) -o game/browser/session1.s -S game/browser/session.cpp

game/map/configuration1.lo: game/map/configuration.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/map/configuration1.lo -c game/map/configuration.cpp

game/map/configuration1.o: game/map/configuration.cpp
	@echo "        Compiling game/map/configuration.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/map/configuration1.o -c game/map/configuration.cpp

game/map/configuration1.s: game/map/configuration.cpp
	$(CXX) $(CXXFLAGS) -o game/map/configuration1.s -S game/map/configuration.cpp

game/map/objectobserver1.lo: game/map/objectobserver.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/map/objectobserver1.lo -c game/map/objectobserver.cpp

game/map/objectobserver1.o: game/map/objectobserver.cpp
	@echo "        Compiling game/map/objectobserver.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/map/objectobserver1.o -c game/map/objectobserver.cpp

game/map/objectobserver1.s: game/map/objectobserver.cpp
	$(CXX) $(CXXFLAGS) -o game/map/objectobserver1.s -S game/map/objectobserver.cpp

game/map/renderer1.lo: game/map/renderer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/map/renderer1.lo -c game/map/renderer.cpp

game/map/renderer1.o: game/map/renderer.cpp
	@echo "        Compiling game/map/renderer.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/map/renderer1.o -c game/map/renderer.cpp

game/map/renderer1.s: game/map/renderer.cpp
	$(CXX) $(CXXFLAGS) -o game/map/renderer1.s -S game/map/renderer.cpp

game/pcc/accountfolder1.lo: game/pcc/accountfolder.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/pcc/accountfolder1.lo -c game/pcc/accountfolder.cpp

game/pcc/accountfolder1.o: game/pcc/accountfolder.cpp
	@echo "        Compiling game/pcc/accountfolder.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/pcc/accountfolder1.o -c game/pcc/accountfolder.cpp

game/pcc/accountfolder1.s: game/pcc/accountfolder.cpp
	$(CXX) $(CXXFLAGS) -o game/pcc/accountfolder1.s -S game/pcc/accountfolder.cpp

game/pcc/browserhandler1.lo: game/pcc/browserhandler.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/pcc/browserhandler1.lo -c game/pcc/browserhandler.cpp

game/pcc/browserhandler1.o: game/pcc/browserhandler.cpp
	@echo "        Compiling game/pcc/browserhandler.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/pcc/browserhandler1.o -c game/pcc/browserhandler.cpp

game/pcc/browserhandler1.s: game/pcc/browserhandler.cpp
	$(CXX) $(CXXFLAGS) -o game/pcc/browserhandler1.s -S game/pcc/browserhandler.cpp

game/pcc/gamefolder1.lo: game/pcc/gamefolder.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/pcc/gamefolder1.lo -c game/pcc/gamefolder.cpp

game/pcc/gamefolder1.o: game/pcc/gamefolder.cpp
	@echo "        Compiling game/pcc/gamefolder.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/pcc/gamefolder1.o -c game/pcc/gamefolder.cpp

game/pcc/gamefolder1.s: game/pcc/gamefolder.cpp
	$(CXX) $(CXXFLAGS) -o game/pcc/gamefolder1.s -S game/pcc/gamefolder.cpp

game/session2.lo: game/session.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/session2.lo -c game/session.cpp

game/session2.o: game/session.cpp
	@echo "        Compiling game/session.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/session2.o -c game/session.cpp

game/session2.s: game/session.cpp
	$(CXX) $(CXXFLAGS) -o game/session2.s -S game/session.cpp

game/spec/hullfunction1.lo: game/spec/hullfunction.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/spec/hullfunction1.lo -c game/spec/hullfunction.cpp

game/spec/hullfunction1.o: game/spec/hullfunction.cpp
	@echo "        Compiling game/spec/hullfunction.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/spec/hullfunction1.o -c game/spec/hullfunction.cpp

game/spec/hullfunction1.s: game/spec/hullfunction.cpp
	$(CXX) $(CXXFLAGS) -o game/spec/hullfunction1.s -S game/spec/hullfunction.cpp

game/stringverifier1.lo: game/stringverifier.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/stringverifier1.lo -c game/stringverifier.cpp

game/stringverifier1.o: game/stringverifier.cpp
	@echo "        Compiling game/stringverifier.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/stringverifier1.o -c game/stringverifier.cpp

game/stringverifier1.s: game/stringverifier.cpp
	$(CXX) $(CXXFLAGS) -o game/stringverifier1.s -S game/stringverifier.cpp

game/turnloader1.lo: game/turnloader.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/turnloader1.lo -c game/turnloader.cpp

game/turnloader1.o: game/turnloader.cpp
	@echo "        Compiling game/turnloader.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/turnloader1.o -c game/turnloader.cpp

game/turnloader1.s: game/turnloader.cpp
	$(CXX) $(CXXFLAGS) -o game/turnloader1.s -S game/turnloader.cpp

game/v3/registrationkey1.lo: game/v3/registrationkey.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/v3/registrationkey1.lo -c game/v3/registrationkey.cpp

game/v3/registrationkey1.o: game/v3/registrationkey.cpp
	@echo "        Compiling game/v3/registrationkey.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/v3/registrationkey1.o -c game/v3/registrationkey.cpp

game/v3/registrationkey1.s: game/v3/registrationkey.cpp
	$(CXX) $(CXXFLAGS) -o game/v3/registrationkey1.s -S game/v3/registrationkey.cpp

game/v3/specificationloader1.lo: game/v3/specificationloader.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/v3/specificationloader1.lo -c game/v3/specificationloader.cpp

game/v3/specificationloader1.o: game/v3/specificationloader.cpp
	@echo "        Compiling game/v3/specificationloader.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/v3/specificationloader1.o -c game/v3/specificationloader.cpp

game/v3/specificationloader1.s: game/v3/specificationloader.cpp
	$(CXX) $(CXXFLAGS) -o game/v3/specificationloader1.s -S game/v3/specificationloader.cpp

game/v3/stringverifier2.lo: game/v3/stringverifier.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/v3/stringverifier2.lo -c game/v3/stringverifier.cpp

game/v3/stringverifier2.o: game/v3/stringverifier.cpp
	@echo "        Compiling game/v3/stringverifier.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/v3/stringverifier2.o -c game/v3/stringverifier.cpp

game/v3/stringverifier2.s: game/v3/stringverifier.cpp
	$(CXX) $(CXXFLAGS) -o game/v3/stringverifier2.s -S game/v3/stringverifier.cpp

game/vcr/object1.lo: game/vcr/object.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o game/vcr/object1.lo -c game/vcr/object.cpp

game/vcr/object1.o: game/vcr/object.cpp
	@echo "        Compiling game/vcr/object.cpp..."
	@$(CXX) $(CXXFLAGS) -o game/vcr/object1.o -c game/vcr/object.cpp

game/vcr/object1.s: game/vcr/object.cpp
	$(CXX) $(CXXFLAGS) -o game/vcr/object1.s -S game/vcr/object.cpp

gamelib: libgamelib.a

gfx/point1.lo: gfx/point.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o gfx/point1.lo -c gfx/point.cpp

gfx/point1.o: gfx/point.cpp
	@echo "        Compiling gfx/point.cpp..."
	@$(CXX) $(CXXFLAGS) -o gfx/point1.o -c gfx/point.cpp

gfx/point1.s: gfx/point.cpp
	$(CXX) $(CXXFLAGS) -o gfx/point1.s -S gfx/point.cpp

gfx/sdl/engine1.lo: gfx/sdl/engine.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o gfx/sdl/engine1.lo -c gfx/sdl/engine.cpp

gfx/sdl/engine1.o: gfx/sdl/engine.cpp
	@echo "        Compiling gfx/sdl/engine.cpp..."
	@$(CXX) $(CXXFLAGS) -o gfx/sdl/engine1.o -c gfx/sdl/engine.cpp

gfx/sdl/engine1.s: gfx/sdl/engine.cpp
	$(CXX) $(CXXFLAGS) -o gfx/sdl/engine1.s -S gfx/sdl/engine.cpp

guilib: libguilib.a

interpreter/context1.lo: interpreter/context.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o interpreter/context1.lo -c interpreter/context.cpp

interpreter/context1.o: interpreter/context.cpp
	@echo "        Compiling interpreter/context.cpp..."
	@$(CXX) $(CXXFLAGS) -o interpreter/context1.o -c interpreter/context.cpp

interpreter/context1.s: interpreter/context.cpp
	$(CXX) $(CXXFLAGS) -o interpreter/context1.s -S interpreter/context.cpp

tags:
	@etags --recurse client game gfx interpreter main tools ui util

test: testsuite
	./testsuite

testsuite.cpp: $(HEADERS_testsuite)
	@echo "        Generating test driver..."
	@$(PERL) $(CXXTESTDIR)/cxxtestgen.pl --gui=TestController --have-eh --error-printer -o $@ $(HEADERS_testsuite)

testsuite.lo: testsuite.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -U_CXXTEST_HAVE_EH -U_CXXTEST_HAVE_STD -O0 -o testsuite.lo -c testsuite.cpp

testsuite.o: testsuite.cpp
	@echo "        Compiling testsuite.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -U_CXXTEST_HAVE_EH -U_CXXTEST_HAVE_STD -O0 -o testsuite.o -c testsuite.cpp

testsuite.s: testsuite.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -U_CXXTEST_HAVE_EH -U_CXXTEST_HAVE_STD -O0 -o testsuite.s -S testsuite.cpp

u/t_client_dialogs_objectselectiondialog.lo: \
    u/t_client_dialogs_objectselectiondialog.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_dialogs_objectselectiondialog.lo -c u/t_client_dialogs_objectselectiondialog.cpp

u/t_client_dialogs_objectselectiondialog.o: \
    u/t_client_dialogs_objectselectiondialog.cpp
	@echo "        Compiling u/t_client_dialogs_objectselectiondialog.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_dialogs_objectselectiondialog.o -c u/t_client_dialogs_objectselectiondialog.cpp

u/t_client_dialogs_objectselectiondialog.s: \
    u/t_client_dialogs_objectselectiondialog.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_dialogs_objectselectiondialog.s -S u/t_client_dialogs_objectselectiondialog.cpp

u/t_client_objectcursorfactory.lo: u/t_client_objectcursorfactory.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectcursorfactory.lo -c u/t_client_objectcursorfactory.cpp

u/t_client_objectcursorfactory.o: u/t_client_objectcursorfactory.cpp
	@echo "        Compiling u/t_client_objectcursorfactory.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectcursorfactory.o -c u/t_client_objectcursorfactory.cpp

u/t_client_objectcursorfactory.s: u/t_client_objectcursorfactory.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectcursorfactory.s -S u/t_client_objectcursorfactory.cpp

u/t_client_objectlistener.lo: u/t_client_objectlistener.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectlistener.lo -c u/t_client_objectlistener.cpp

u/t_client_objectlistener.o: u/t_client_objectlistener.cpp
	@echo "        Compiling u/t_client_objectlistener.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectlistener.o -c u/t_client_objectlistener.cpp

u/t_client_objectlistener.s: u/t_client_objectlistener.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_objectlistener.s -S u/t_client_objectlistener.cpp

u/t_client_si_contextprovider.lo: u/t_client_si_contextprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_contextprovider.lo -c u/t_client_si_contextprovider.cpp

u/t_client_si_contextprovider.o: u/t_client_si_contextprovider.cpp
	@echo "        Compiling u/t_client_si_contextprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_contextprovider.o -c u/t_client_si_contextprovider.cpp

u/t_client_si_contextprovider.s: u/t_client_si_contextprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_contextprovider.s -S u/t_client_si_contextprovider.cpp

u/t_client_si_control.lo: u/t_client_si_control.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_control.lo -c u/t_client_si_control.cpp

u/t_client_si_control.o: u/t_client_si_control.cpp
	@echo "        Compiling u/t_client_si_control.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_control.o -c u/t_client_si_control.cpp

u/t_client_si_control.s: u/t_client_si_control.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_control.s -S u/t_client_si_control.cpp

u/t_client_si_requestlink1.lo: u/t_client_si_requestlink1.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink1.lo -c u/t_client_si_requestlink1.cpp

u/t_client_si_requestlink1.o: u/t_client_si_requestlink1.cpp
	@echo "        Compiling u/t_client_si_requestlink1.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink1.o -c u/t_client_si_requestlink1.cpp

u/t_client_si_requestlink1.s: u/t_client_si_requestlink1.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink1.s -S u/t_client_si_requestlink1.cpp

u/t_client_si_requestlink2.lo: u/t_client_si_requestlink2.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink2.lo -c u/t_client_si_requestlink2.cpp

u/t_client_si_requestlink2.o: u/t_client_si_requestlink2.cpp
	@echo "        Compiling u/t_client_si_requestlink2.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink2.o -c u/t_client_si_requestlink2.cpp

u/t_client_si_requestlink2.s: u/t_client_si_requestlink2.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_requestlink2.s -S u/t_client_si_requestlink2.cpp

u/t_client_si_usertask.lo: u/t_client_si_usertask.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_usertask.lo -c u/t_client_si_usertask.cpp

u/t_client_si_usertask.o: u/t_client_si_usertask.cpp
	@echo "        Compiling u/t_client_si_usertask.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_usertask.o -c u/t_client_si_usertask.cpp

u/t_client_si_usertask.s: u/t_client_si_usertask.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_client_si_usertask.s -S u/t_client_si_usertask.cpp

u/t_game_browser_account.lo: u/t_game_browser_account.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_account.lo -c u/t_game_browser_account.cpp

u/t_game_browser_account.o: u/t_game_browser_account.cpp
	@echo "        Compiling u/t_game_browser_account.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_account.o -c u/t_game_browser_account.cpp

u/t_game_browser_account.s: u/t_game_browser_account.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_account.s -S u/t_game_browser_account.cpp

u/t_game_browser_folder.lo: u/t_game_browser_folder.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_folder.lo -c u/t_game_browser_folder.cpp

u/t_game_browser_folder.o: u/t_game_browser_folder.cpp
	@echo "        Compiling u/t_game_browser_folder.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_folder.o -c u/t_game_browser_folder.cpp

u/t_game_browser_folder.s: u/t_game_browser_folder.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_folder.s -S u/t_game_browser_folder.cpp

u/t_game_browser_handler.lo: u/t_game_browser_handler.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handler.lo -c u/t_game_browser_handler.cpp

u/t_game_browser_handler.o: u/t_game_browser_handler.cpp
	@echo "        Compiling u/t_game_browser_handler.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handler.o -c u/t_game_browser_handler.cpp

u/t_game_browser_handler.s: u/t_game_browser_handler.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handler.s -S u/t_game_browser_handler.cpp

u/t_game_browser_handlerlist.lo: u/t_game_browser_handlerlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handlerlist.lo -c u/t_game_browser_handlerlist.cpp

u/t_game_browser_handlerlist.o: u/t_game_browser_handlerlist.cpp
	@echo "        Compiling u/t_game_browser_handlerlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handlerlist.o -c u/t_game_browser_handlerlist.cpp

u/t_game_browser_handlerlist.s: u/t_game_browser_handlerlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_handlerlist.s -S u/t_game_browser_handlerlist.cpp

u/t_game_browser_unsupportedaccountfolder.lo: \
    u/t_game_browser_unsupportedaccountfolder.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_unsupportedaccountfolder.lo -c u/t_game_browser_unsupportedaccountfolder.cpp

u/t_game_browser_unsupportedaccountfolder.o: \
    u/t_game_browser_unsupportedaccountfolder.cpp
	@echo "        Compiling u/t_game_browser_unsupportedaccountfolder.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_unsupportedaccountfolder.o -c u/t_game_browser_unsupportedaccountfolder.cpp

u/t_game_browser_unsupportedaccountfolder.s: \
    u/t_game_browser_unsupportedaccountfolder.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_browser_unsupportedaccountfolder.s -S u/t_game_browser_unsupportedaccountfolder.cpp

u/t_game_cargospec.lo: u/t_game_cargospec.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_cargospec.lo -c u/t_game_cargospec.cpp

u/t_game_cargospec.o: u/t_game_cargospec.cpp
	@echo "        Compiling u/t_game_cargospec.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_cargospec.o -c u/t_game_cargospec.cpp

u/t_game_cargospec.s: u/t_game_cargospec.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_cargospec.s -S u/t_game_cargospec.cpp

u/t_game_config_bitsetvalueparser.lo: u/t_game_config_bitsetvalueparser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_bitsetvalueparser.lo -c u/t_game_config_bitsetvalueparser.cpp

u/t_game_config_bitsetvalueparser.o: u/t_game_config_bitsetvalueparser.cpp
	@echo "        Compiling u/t_game_config_bitsetvalueparser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_bitsetvalueparser.o -c u/t_game_config_bitsetvalueparser.cpp

u/t_game_config_bitsetvalueparser.s: u/t_game_config_bitsetvalueparser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_bitsetvalueparser.s -S u/t_game_config_bitsetvalueparser.cpp

u/t_game_config_booleanvalueparser.lo: \
    u/t_game_config_booleanvalueparser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_booleanvalueparser.lo -c u/t_game_config_booleanvalueparser.cpp

u/t_game_config_booleanvalueparser.o: u/t_game_config_booleanvalueparser.cpp
	@echo "        Compiling u/t_game_config_booleanvalueparser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_booleanvalueparser.o -c u/t_game_config_booleanvalueparser.cpp

u/t_game_config_booleanvalueparser.s: u/t_game_config_booleanvalueparser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_booleanvalueparser.s -S u/t_game_config_booleanvalueparser.cpp

u/t_game_config_configuration.lo: u/t_game_config_configuration.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configuration.lo -c u/t_game_config_configuration.cpp

u/t_game_config_configuration.o: u/t_game_config_configuration.cpp
	@echo "        Compiling u/t_game_config_configuration.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configuration.o -c u/t_game_config_configuration.cpp

u/t_game_config_configuration.s: u/t_game_config_configuration.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configuration.s -S u/t_game_config_configuration.cpp

u/t_game_config_configurationoption.lo: \
    u/t_game_config_configurationoption.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configurationoption.lo -c u/t_game_config_configurationoption.cpp

u/t_game_config_configurationoption.o: \
    u/t_game_config_configurationoption.cpp
	@echo "        Compiling u/t_game_config_configurationoption.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configurationoption.o -c u/t_game_config_configurationoption.cpp

u/t_game_config_configurationoption.s: \
    u/t_game_config_configurationoption.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_configurationoption.s -S u/t_game_config_configurationoption.cpp

u/t_game_config_costarrayoption.lo: u/t_game_config_costarrayoption.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_costarrayoption.lo -c u/t_game_config_costarrayoption.cpp

u/t_game_config_costarrayoption.o: u/t_game_config_costarrayoption.cpp
	@echo "        Compiling u/t_game_config_costarrayoption.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_costarrayoption.o -c u/t_game_config_costarrayoption.cpp

u/t_game_config_costarrayoption.s: u/t_game_config_costarrayoption.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_costarrayoption.s -S u/t_game_config_costarrayoption.cpp

u/t_game_config_hostconfiguration.lo: u/t_game_config_hostconfiguration.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_hostconfiguration.lo -c u/t_game_config_hostconfiguration.cpp

u/t_game_config_hostconfiguration.o: u/t_game_config_hostconfiguration.cpp
	@echo "        Compiling u/t_game_config_hostconfiguration.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_hostconfiguration.o -c u/t_game_config_hostconfiguration.cpp

u/t_game_config_hostconfiguration.s: u/t_game_config_hostconfiguration.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_hostconfiguration.s -S u/t_game_config_hostconfiguration.cpp

u/t_game_config_integervalueparser.lo: \
    u/t_game_config_integervalueparser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_integervalueparser.lo -c u/t_game_config_integervalueparser.cpp

u/t_game_config_integervalueparser.o: u/t_game_config_integervalueparser.cpp
	@echo "        Compiling u/t_game_config_integervalueparser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_integervalueparser.o -c u/t_game_config_integervalueparser.cpp

u/t_game_config_integervalueparser.s: u/t_game_config_integervalueparser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_integervalueparser.s -S u/t_game_config_integervalueparser.cpp

u/t_game_config_valueparser.lo: u/t_game_config_valueparser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_valueparser.lo -c u/t_game_config_valueparser.cpp

u/t_game_config_valueparser.o: u/t_game_config_valueparser.cpp
	@echo "        Compiling u/t_game_config_valueparser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_valueparser.o -c u/t_game_config_valueparser.cpp

u/t_game_config_valueparser.s: u/t_game_config_valueparser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_config_valueparser.s -S u/t_game_config_valueparser.cpp

u/t_game_element.lo: u/t_game_element.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_element.lo -c u/t_game_element.cpp

u/t_game_element.o: u/t_game_element.cpp
	@echo "        Compiling u/t_game_element.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_element.o -c u/t_game_element.cpp

u/t_game_element.s: u/t_game_element.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_element.s -S u/t_game_element.cpp

u/t_game_exception.lo: u/t_game_exception.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_exception.lo -c u/t_game_exception.cpp

u/t_game_exception.o: u/t_game_exception.cpp
	@echo "        Compiling u/t_game_exception.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_exception.o -c u/t_game_exception.cpp

u/t_game_exception.s: u/t_game_exception.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_exception.s -S u/t_game_exception.cpp

u/t_game_experiencelevelset.lo: u/t_game_experiencelevelset.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_experiencelevelset.lo -c u/t_game_experiencelevelset.cpp

u/t_game_experiencelevelset.o: u/t_game_experiencelevelset.cpp
	@echo "        Compiling u/t_game_experiencelevelset.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_experiencelevelset.o -c u/t_game_experiencelevelset.cpp

u/t_game_experiencelevelset.s: u/t_game_experiencelevelset.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_experiencelevelset.s -S u/t_game_experiencelevelset.cpp

u/t_game_extracontainer.lo: u/t_game_extracontainer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extracontainer.lo -c u/t_game_extracontainer.cpp

u/t_game_extracontainer.o: u/t_game_extracontainer.cpp
	@echo "        Compiling u/t_game_extracontainer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extracontainer.o -c u/t_game_extracontainer.cpp

u/t_game_extracontainer.s: u/t_game_extracontainer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extracontainer.s -S u/t_game_extracontainer.cpp

u/t_game_extraidentifier.lo: u/t_game_extraidentifier.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extraidentifier.lo -c u/t_game_extraidentifier.cpp

u/t_game_extraidentifier.o: u/t_game_extraidentifier.cpp
	@echo "        Compiling u/t_game_extraidentifier.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extraidentifier.o -c u/t_game_extraidentifier.cpp

u/t_game_extraidentifier.s: u/t_game_extraidentifier.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_extraidentifier.s -S u/t_game_extraidentifier.cpp

u/t_game_game.lo: u/t_game_game.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_game.lo -c u/t_game_game.cpp

u/t_game_game.o: u/t_game_game.cpp
	@echo "        Compiling u/t_game_game.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_game.o -c u/t_game_game.cpp

u/t_game_game.s: u/t_game_game.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_game.s -S u/t_game_game.cpp

u/t_game_historyturn.lo: u/t_game_historyturn.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturn.lo -c u/t_game_historyturn.cpp

u/t_game_historyturn.o: u/t_game_historyturn.cpp
	@echo "        Compiling u/t_game_historyturn.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturn.o -c u/t_game_historyturn.cpp

u/t_game_historyturn.s: u/t_game_historyturn.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturn.s -S u/t_game_historyturn.cpp

u/t_game_historyturnlist.lo: u/t_game_historyturnlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturnlist.lo -c u/t_game_historyturnlist.cpp

u/t_game_historyturnlist.o: u/t_game_historyturnlist.cpp
	@echo "        Compiling u/t_game_historyturnlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturnlist.o -c u/t_game_historyturnlist.cpp

u/t_game_historyturnlist.s: u/t_game_historyturnlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_historyturnlist.s -S u/t_game_historyturnlist.cpp

u/t_game_hostversion.lo: u/t_game_hostversion.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_hostversion.lo -c u/t_game_hostversion.cpp

u/t_game_hostversion.o: u/t_game_hostversion.cpp
	@echo "        Compiling u/t_game_hostversion.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_hostversion.o -c u/t_game_hostversion.cpp

u/t_game_hostversion.s: u/t_game_hostversion.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_hostversion.s -S u/t_game_hostversion.cpp

u/t_game_interface_iteratorprovider.lo: \
    u/t_game_interface_iteratorprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_iteratorprovider.lo -c u/t_game_interface_iteratorprovider.cpp

u/t_game_interface_iteratorprovider.o: \
    u/t_game_interface_iteratorprovider.cpp
	@echo "        Compiling u/t_game_interface_iteratorprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_iteratorprovider.o -c u/t_game_interface_iteratorprovider.cpp

u/t_game_interface_iteratorprovider.s: \
    u/t_game_interface_iteratorprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_iteratorprovider.s -S u/t_game_interface_iteratorprovider.cpp

u/t_game_interface_richtextfunctions.lo: \
    u/t_game_interface_richtextfunctions.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_richtextfunctions.lo -c u/t_game_interface_richtextfunctions.cpp

u/t_game_interface_richtextfunctions.o: \
    u/t_game_interface_richtextfunctions.cpp
	@echo "        Compiling u/t_game_interface_richtextfunctions.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_richtextfunctions.o -c u/t_game_interface_richtextfunctions.cpp

u/t_game_interface_richtextfunctions.s: \
    u/t_game_interface_richtextfunctions.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_richtextfunctions.s -S u/t_game_interface_richtextfunctions.cpp

u/t_game_interface_userinterfaceproperty.lo: \
    u/t_game_interface_userinterfaceproperty.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfaceproperty.lo -c u/t_game_interface_userinterfaceproperty.cpp

u/t_game_interface_userinterfaceproperty.o: \
    u/t_game_interface_userinterfaceproperty.cpp
	@echo "        Compiling u/t_game_interface_userinterfaceproperty.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfaceproperty.o -c u/t_game_interface_userinterfaceproperty.cpp

u/t_game_interface_userinterfaceproperty.s: \
    u/t_game_interface_userinterfaceproperty.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfaceproperty.s -S u/t_game_interface_userinterfaceproperty.cpp

u/t_game_interface_userinterfacepropertyaccessor.lo: \
    u/t_game_interface_userinterfacepropertyaccessor.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertyaccessor.lo -c u/t_game_interface_userinterfacepropertyaccessor.cpp

u/t_game_interface_userinterfacepropertyaccessor.o: \
    u/t_game_interface_userinterfacepropertyaccessor.cpp
	@echo "        Compiling u/t_game_interface_userinterfacepropertyaccessor.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertyaccessor.o -c u/t_game_interface_userinterfacepropertyaccessor.cpp

u/t_game_interface_userinterfacepropertyaccessor.s: \
    u/t_game_interface_userinterfacepropertyaccessor.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertyaccessor.s -S u/t_game_interface_userinterfacepropertyaccessor.cpp

u/t_game_interface_userinterfacepropertystack.lo: \
    u/t_game_interface_userinterfacepropertystack.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertystack.lo -c u/t_game_interface_userinterfacepropertystack.cpp

u/t_game_interface_userinterfacepropertystack.o: \
    u/t_game_interface_userinterfacepropertystack.cpp
	@echo "        Compiling u/t_game_interface_userinterfacepropertystack.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertystack.o -c u/t_game_interface_userinterfacepropertystack.cpp

u/t_game_interface_userinterfacepropertystack.s: \
    u/t_game_interface_userinterfacepropertystack.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interface_userinterfacepropertystack.s -S u/t_game_interface_userinterfacepropertystack.cpp

u/t_game_interpreterinterface.lo: u/t_game_interpreterinterface.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interpreterinterface.lo -c u/t_game_interpreterinterface.cpp

u/t_game_interpreterinterface.o: u/t_game_interpreterinterface.cpp
	@echo "        Compiling u/t_game_interpreterinterface.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interpreterinterface.o -c u/t_game_interpreterinterface.cpp

u/t_game_interpreterinterface.s: u/t_game_interpreterinterface.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_interpreterinterface.s -S u/t_game_interpreterinterface.cpp

u/t_game_map_circularobject.lo: u/t_game_map_circularobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_circularobject.lo -c u/t_game_map_circularobject.cpp

u/t_game_map_circularobject.o: u/t_game_map_circularobject.cpp
	@echo "        Compiling u/t_game_map_circularobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_circularobject.o -c u/t_game_map_circularobject.cpp

u/t_game_map_circularobject.s: u/t_game_map_circularobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_circularobject.s -S u/t_game_map_circularobject.cpp

u/t_game_map_configuration.lo: u/t_game_map_configuration.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_configuration.lo -c u/t_game_map_configuration.cpp

u/t_game_map_configuration.o: u/t_game_map_configuration.cpp
	@echo "        Compiling u/t_game_map_configuration.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_configuration.o -c u/t_game_map_configuration.cpp

u/t_game_map_configuration.s: u/t_game_map_configuration.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_configuration.s -S u/t_game_map_configuration.cpp

u/t_game_map_mapobject.lo: u/t_game_map_mapobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_mapobject.lo -c u/t_game_map_mapobject.cpp

u/t_game_map_mapobject.o: u/t_game_map_mapobject.cpp
	@echo "        Compiling u/t_game_map_mapobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_mapobject.o -c u/t_game_map_mapobject.cpp

u/t_game_map_mapobject.s: u/t_game_map_mapobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_mapobject.s -S u/t_game_map_mapobject.cpp

u/t_game_map_object.lo: u/t_game_map_object.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_object.lo -c u/t_game_map_object.cpp

u/t_game_map_object.o: u/t_game_map_object.cpp
	@echo "        Compiling u/t_game_map_object.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_object.o -c u/t_game_map_object.cpp

u/t_game_map_object.s: u/t_game_map_object.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_object.s -S u/t_game_map_object.cpp

u/t_game_map_objectcursor.lo: u/t_game_map_objectcursor.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectcursor.lo -c u/t_game_map_objectcursor.cpp

u/t_game_map_objectcursor.o: u/t_game_map_objectcursor.cpp
	@echo "        Compiling u/t_game_map_objectcursor.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectcursor.o -c u/t_game_map_objectcursor.cpp

u/t_game_map_objectcursor.s: u/t_game_map_objectcursor.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectcursor.s -S u/t_game_map_objectcursor.cpp

u/t_game_map_objectlist.lo: u/t_game_map_objectlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectlist.lo -c u/t_game_map_objectlist.cpp

u/t_game_map_objectlist.o: u/t_game_map_objectlist.cpp
	@echo "        Compiling u/t_game_map_objectlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectlist.o -c u/t_game_map_objectlist.cpp

u/t_game_map_objectlist.s: u/t_game_map_objectlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectlist.s -S u/t_game_map_objectlist.cpp

u/t_game_map_objectreference.lo: u/t_game_map_objectreference.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectreference.lo -c u/t_game_map_objectreference.cpp

u/t_game_map_objectreference.o: u/t_game_map_objectreference.cpp
	@echo "        Compiling u/t_game_map_objectreference.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectreference.o -c u/t_game_map_objectreference.cpp

u/t_game_map_objectreference.s: u/t_game_map_objectreference.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_objectreference.s -S u/t_game_map_objectreference.cpp

u/t_game_map_point.lo: u/t_game_map_point.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_point.lo -c u/t_game_map_point.cpp

u/t_game_map_point.o: u/t_game_map_point.cpp
	@echo "        Compiling u/t_game_map_point.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_point.o -c u/t_game_map_point.cpp

u/t_game_map_point.s: u/t_game_map_point.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_point.s -S u/t_game_map_point.cpp

u/t_game_map_viewport.lo: u/t_game_map_viewport.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_viewport.lo -c u/t_game_map_viewport.cpp

u/t_game_map_viewport.o: u/t_game_map_viewport.cpp
	@echo "        Compiling u/t_game_map_viewport.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_viewport.o -c u/t_game_map_viewport.cpp

u/t_game_map_viewport.s: u/t_game_map_viewport.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_map_viewport.s -S u/t_game_map_viewport.cpp

u/t_game_msg_mailbox.lo: u/t_game_msg_mailbox.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_msg_mailbox.lo -c u/t_game_msg_mailbox.cpp

u/t_game_msg_mailbox.o: u/t_game_msg_mailbox.cpp
	@echo "        Compiling u/t_game_msg_mailbox.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_msg_mailbox.o -c u/t_game_msg_mailbox.cpp

u/t_game_msg_mailbox.s: u/t_game_msg_mailbox.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_msg_mailbox.s -S u/t_game_msg_mailbox.cpp

u/t_game_parser_datainterface.lo: u/t_game_parser_datainterface.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_datainterface.lo -c u/t_game_parser_datainterface.cpp

u/t_game_parser_datainterface.o: u/t_game_parser_datainterface.cpp
	@echo "        Compiling u/t_game_parser_datainterface.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_datainterface.o -c u/t_game_parser_datainterface.cpp

u/t_game_parser_datainterface.s: u/t_game_parser_datainterface.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_datainterface.s -S u/t_game_parser_datainterface.cpp

u/t_game_parser_messagetemplate.lo: u/t_game_parser_messagetemplate.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_messagetemplate.lo -c u/t_game_parser_messagetemplate.cpp

u/t_game_parser_messagetemplate.o: u/t_game_parser_messagetemplate.cpp
	@echo "        Compiling u/t_game_parser_messagetemplate.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_messagetemplate.o -c u/t_game_parser_messagetemplate.cpp

u/t_game_parser_messagetemplate.s: u/t_game_parser_messagetemplate.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_parser_messagetemplate.s -S u/t_game_parser_messagetemplate.cpp

u/t_game_player.lo: u/t_game_player.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_player.lo -c u/t_game_player.cpp

u/t_game_player.o: u/t_game_player.cpp
	@echo "        Compiling u/t_game_player.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_player.o -c u/t_game_player.cpp

u/t_game_player.s: u/t_game_player.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_player.s -S u/t_game_player.cpp

u/t_game_playerarray.lo: u/t_game_playerarray.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerarray.lo -c u/t_game_playerarray.cpp

u/t_game_playerarray.o: u/t_game_playerarray.cpp
	@echo "        Compiling u/t_game_playerarray.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerarray.o -c u/t_game_playerarray.cpp

u/t_game_playerarray.s: u/t_game_playerarray.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerarray.s -S u/t_game_playerarray.cpp

u/t_game_playerbitmatrix.lo: u/t_game_playerbitmatrix.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerbitmatrix.lo -c u/t_game_playerbitmatrix.cpp

u/t_game_playerbitmatrix.o: u/t_game_playerbitmatrix.cpp
	@echo "        Compiling u/t_game_playerbitmatrix.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerbitmatrix.o -c u/t_game_playerbitmatrix.cpp

u/t_game_playerbitmatrix.s: u/t_game_playerbitmatrix.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_playerbitmatrix.s -S u/t_game_playerbitmatrix.cpp

u/t_game_registrationkey.lo: u/t_game_registrationkey.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_registrationkey.lo -c u/t_game_registrationkey.cpp

u/t_game_registrationkey.o: u/t_game_registrationkey.cpp
	@echo "        Compiling u/t_game_registrationkey.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_registrationkey.o -c u/t_game_registrationkey.cpp

u/t_game_registrationkey.s: u/t_game_registrationkey.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_registrationkey.s -S u/t_game_registrationkey.cpp

u/t_game_spec_component.lo: u/t_game_spec_component.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_component.lo -c u/t_game_spec_component.cpp

u/t_game_spec_component.o: u/t_game_spec_component.cpp
	@echo "        Compiling u/t_game_spec_component.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_component.o -c u/t_game_spec_component.cpp

u/t_game_spec_component.s: u/t_game_spec_component.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_component.s -S u/t_game_spec_component.cpp

u/t_game_spec_componentnameprovider.lo: \
    u/t_game_spec_componentnameprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentnameprovider.lo -c u/t_game_spec_componentnameprovider.cpp

u/t_game_spec_componentnameprovider.o: \
    u/t_game_spec_componentnameprovider.cpp
	@echo "        Compiling u/t_game_spec_componentnameprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentnameprovider.o -c u/t_game_spec_componentnameprovider.cpp

u/t_game_spec_componentnameprovider.s: \
    u/t_game_spec_componentnameprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentnameprovider.s -S u/t_game_spec_componentnameprovider.cpp

u/t_game_spec_componentvector.lo: u/t_game_spec_componentvector.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentvector.lo -c u/t_game_spec_componentvector.cpp

u/t_game_spec_componentvector.o: u/t_game_spec_componentvector.cpp
	@echo "        Compiling u/t_game_spec_componentvector.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentvector.o -c u/t_game_spec_componentvector.cpp

u/t_game_spec_componentvector.s: u/t_game_spec_componentvector.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_componentvector.s -S u/t_game_spec_componentvector.cpp

u/t_game_spec_cost.lo: u/t_game_spec_cost.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_cost.lo -c u/t_game_spec_cost.cpp

u/t_game_spec_cost.o: u/t_game_spec_cost.cpp
	@echo "        Compiling u/t_game_spec_cost.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_cost.o -c u/t_game_spec_cost.cpp

u/t_game_spec_cost.s: u/t_game_spec_cost.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_cost.s -S u/t_game_spec_cost.cpp

u/t_game_spec_engine.lo: u/t_game_spec_engine.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_engine.lo -c u/t_game_spec_engine.cpp

u/t_game_spec_engine.o: u/t_game_spec_engine.cpp
	@echo "        Compiling u/t_game_spec_engine.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_engine.o -c u/t_game_spec_engine.cpp

u/t_game_spec_engine.s: u/t_game_spec_engine.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_engine.s -S u/t_game_spec_engine.cpp

u/t_game_spec_friendlycode.lo: u/t_game_spec_friendlycode.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycode.lo -c u/t_game_spec_friendlycode.cpp

u/t_game_spec_friendlycode.o: u/t_game_spec_friendlycode.cpp
	@echo "        Compiling u/t_game_spec_friendlycode.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycode.o -c u/t_game_spec_friendlycode.cpp

u/t_game_spec_friendlycode.s: u/t_game_spec_friendlycode.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycode.s -S u/t_game_spec_friendlycode.cpp

u/t_game_spec_friendlycodelist.lo: u/t_game_spec_friendlycodelist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycodelist.lo -c u/t_game_spec_friendlycodelist.cpp

u/t_game_spec_friendlycodelist.o: u/t_game_spec_friendlycodelist.cpp
	@echo "        Compiling u/t_game_spec_friendlycodelist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycodelist.o -c u/t_game_spec_friendlycodelist.cpp

u/t_game_spec_friendlycodelist.s: u/t_game_spec_friendlycodelist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_friendlycodelist.s -S u/t_game_spec_friendlycodelist.cpp

u/t_game_spec_hullfunctionlist.lo: u/t_game_spec_hullfunctionlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_hullfunctionlist.lo -c u/t_game_spec_hullfunctionlist.cpp

u/t_game_spec_hullfunctionlist.o: u/t_game_spec_hullfunctionlist.cpp
	@echo "        Compiling u/t_game_spec_hullfunctionlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_hullfunctionlist.o -c u/t_game_spec_hullfunctionlist.cpp

u/t_game_spec_hullfunctionlist.s: u/t_game_spec_hullfunctionlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_hullfunctionlist.s -S u/t_game_spec_hullfunctionlist.cpp

u/t_game_spec_missionlist.lo: u/t_game_spec_missionlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_missionlist.lo -c u/t_game_spec_missionlist.cpp

u/t_game_spec_missionlist.o: u/t_game_spec_missionlist.cpp
	@echo "        Compiling u/t_game_spec_missionlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_missionlist.o -c u/t_game_spec_missionlist.cpp

u/t_game_spec_missionlist.s: u/t_game_spec_missionlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_spec_missionlist.s -S u/t_game_spec_missionlist.cpp

u/t_game_specificationloader.lo: u/t_game_specificationloader.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_specificationloader.lo -c u/t_game_specificationloader.cpp

u/t_game_specificationloader.o: u/t_game_specificationloader.cpp
	@echo "        Compiling u/t_game_specificationloader.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_specificationloader.o -c u/t_game_specificationloader.cpp

u/t_game_specificationloader.s: u/t_game_specificationloader.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_specificationloader.s -S u/t_game_specificationloader.cpp

u/t_game_stringverifier.lo: u/t_game_stringverifier.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_stringverifier.lo -c u/t_game_stringverifier.cpp

u/t_game_stringverifier.o: u/t_game_stringverifier.cpp
	@echo "        Compiling u/t_game_stringverifier.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_stringverifier.o -c u/t_game_stringverifier.cpp

u/t_game_stringverifier.s: u/t_game_stringverifier.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_stringverifier.s -S u/t_game_stringverifier.cpp

u/t_game_tables_basemissionname.lo: u/t_game_tables_basemissionname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_basemissionname.lo -c u/t_game_tables_basemissionname.cpp

u/t_game_tables_basemissionname.o: u/t_game_tables_basemissionname.cpp
	@echo "        Compiling u/t_game_tables_basemissionname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_basemissionname.o -c u/t_game_tables_basemissionname.cpp

u/t_game_tables_basemissionname.s: u/t_game_tables_basemissionname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_basemissionname.s -S u/t_game_tables_basemissionname.cpp

u/t_game_tables_happinesschangename.lo: \
    u/t_game_tables_happinesschangename.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinesschangename.lo -c u/t_game_tables_happinesschangename.cpp

u/t_game_tables_happinesschangename.o: \
    u/t_game_tables_happinesschangename.cpp
	@echo "        Compiling u/t_game_tables_happinesschangename.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinesschangename.o -c u/t_game_tables_happinesschangename.cpp

u/t_game_tables_happinesschangename.s: \
    u/t_game_tables_happinesschangename.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinesschangename.s -S u/t_game_tables_happinesschangename.cpp

u/t_game_tables_happinessname.lo: u/t_game_tables_happinessname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinessname.lo -c u/t_game_tables_happinessname.cpp

u/t_game_tables_happinessname.o: u/t_game_tables_happinessname.cpp
	@echo "        Compiling u/t_game_tables_happinessname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinessname.o -c u/t_game_tables_happinessname.cpp

u/t_game_tables_happinessname.s: u/t_game_tables_happinessname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_happinessname.s -S u/t_game_tables_happinessname.cpp

u/t_game_tables_headingname.lo: u/t_game_tables_headingname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_headingname.lo -c u/t_game_tables_headingname.cpp

u/t_game_tables_headingname.o: u/t_game_tables_headingname.cpp
	@echo "        Compiling u/t_game_tables_headingname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_headingname.o -c u/t_game_tables_headingname.cpp

u/t_game_tables_headingname.s: u/t_game_tables_headingname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_headingname.s -S u/t_game_tables_headingname.cpp

u/t_game_tables_industrylevel.lo: u/t_game_tables_industrylevel.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_industrylevel.lo -c u/t_game_tables_industrylevel.cpp

u/t_game_tables_industrylevel.o: u/t_game_tables_industrylevel.cpp
	@echo "        Compiling u/t_game_tables_industrylevel.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_industrylevel.o -c u/t_game_tables_industrylevel.cpp

u/t_game_tables_industrylevel.s: u/t_game_tables_industrylevel.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_industrylevel.s -S u/t_game_tables_industrylevel.cpp

u/t_game_tables_ionstormclassname.lo: u/t_game_tables_ionstormclassname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_ionstormclassname.lo -c u/t_game_tables_ionstormclassname.cpp

u/t_game_tables_ionstormclassname.o: u/t_game_tables_ionstormclassname.cpp
	@echo "        Compiling u/t_game_tables_ionstormclassname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_ionstormclassname.o -c u/t_game_tables_ionstormclassname.cpp

u/t_game_tables_ionstormclassname.s: u/t_game_tables_ionstormclassname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_ionstormclassname.s -S u/t_game_tables_ionstormclassname.cpp

u/t_game_tables_mineraldensityclassname.lo: \
    u/t_game_tables_mineraldensityclassname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineraldensityclassname.lo -c u/t_game_tables_mineraldensityclassname.cpp

u/t_game_tables_mineraldensityclassname.o: \
    u/t_game_tables_mineraldensityclassname.cpp
	@echo "        Compiling u/t_game_tables_mineraldensityclassname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineraldensityclassname.o -c u/t_game_tables_mineraldensityclassname.cpp

u/t_game_tables_mineraldensityclassname.s: \
    u/t_game_tables_mineraldensityclassname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineraldensityclassname.s -S u/t_game_tables_mineraldensityclassname.cpp

u/t_game_tables_mineralmassclassname.lo: \
    u/t_game_tables_mineralmassclassname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineralmassclassname.lo -c u/t_game_tables_mineralmassclassname.cpp

u/t_game_tables_mineralmassclassname.o: \
    u/t_game_tables_mineralmassclassname.cpp
	@echo "        Compiling u/t_game_tables_mineralmassclassname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineralmassclassname.o -c u/t_game_tables_mineralmassclassname.cpp

u/t_game_tables_mineralmassclassname.s: \
    u/t_game_tables_mineralmassclassname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_mineralmassclassname.s -S u/t_game_tables_mineralmassclassname.cpp

u/t_game_tables_nativegovernmentname.lo: \
    u/t_game_tables_nativegovernmentname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativegovernmentname.lo -c u/t_game_tables_nativegovernmentname.cpp

u/t_game_tables_nativegovernmentname.o: \
    u/t_game_tables_nativegovernmentname.cpp
	@echo "        Compiling u/t_game_tables_nativegovernmentname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativegovernmentname.o -c u/t_game_tables_nativegovernmentname.cpp

u/t_game_tables_nativegovernmentname.s: \
    u/t_game_tables_nativegovernmentname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativegovernmentname.s -S u/t_game_tables_nativegovernmentname.cpp

u/t_game_tables_nativeracename.lo: u/t_game_tables_nativeracename.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativeracename.lo -c u/t_game_tables_nativeracename.cpp

u/t_game_tables_nativeracename.o: u/t_game_tables_nativeracename.cpp
	@echo "        Compiling u/t_game_tables_nativeracename.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativeracename.o -c u/t_game_tables_nativeracename.cpp

u/t_game_tables_nativeracename.s: u/t_game_tables_nativeracename.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_nativeracename.s -S u/t_game_tables_nativeracename.cpp

u/t_game_tables_temperaturename.lo: u/t_game_tables_temperaturename.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_temperaturename.lo -c u/t_game_tables_temperaturename.cpp

u/t_game_tables_temperaturename.o: u/t_game_tables_temperaturename.cpp
	@echo "        Compiling u/t_game_tables_temperaturename.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_temperaturename.o -c u/t_game_tables_temperaturename.cpp

u/t_game_tables_temperaturename.s: u/t_game_tables_temperaturename.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_temperaturename.s -S u/t_game_tables_temperaturename.cpp

u/t_game_tables_wormholestabilityname.lo: \
    u/t_game_tables_wormholestabilityname.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_wormholestabilityname.lo -c u/t_game_tables_wormholestabilityname.cpp

u/t_game_tables_wormholestabilityname.o: \
    u/t_game_tables_wormholestabilityname.cpp
	@echo "        Compiling u/t_game_tables_wormholestabilityname.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_wormholestabilityname.o -c u/t_game_tables_wormholestabilityname.cpp

u/t_game_tables_wormholestabilityname.s: \
    u/t_game_tables_wormholestabilityname.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_tables_wormholestabilityname.s -S u/t_game_tables_wormholestabilityname.cpp

u/t_game_teamsettings.lo: u/t_game_teamsettings.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_teamsettings.lo -c u/t_game_teamsettings.cpp

u/t_game_teamsettings.o: u/t_game_teamsettings.cpp
	@echo "        Compiling u/t_game_teamsettings.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_teamsettings.o -c u/t_game_teamsettings.cpp

u/t_game_teamsettings.s: u/t_game_teamsettings.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_teamsettings.s -S u/t_game_teamsettings.cpp

u/t_game_timestamp.lo: u/t_game_timestamp.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_timestamp.lo -c u/t_game_timestamp.cpp

u/t_game_timestamp.o: u/t_game_timestamp.cpp
	@echo "        Compiling u/t_game_timestamp.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_timestamp.o -c u/t_game_timestamp.cpp

u/t_game_timestamp.s: u/t_game_timestamp.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_timestamp.s -S u/t_game_timestamp.cpp

u/t_game_turnloader.lo: u/t_game_turnloader.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_turnloader.lo -c u/t_game_turnloader.cpp

u/t_game_turnloader.o: u/t_game_turnloader.cpp
	@echo "        Compiling u/t_game_turnloader.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_turnloader.o -c u/t_game_turnloader.cpp

u/t_game_turnloader.s: u/t_game_turnloader.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_turnloader.s -S u/t_game_turnloader.cpp

u/t_game_types.lo: u/t_game_types.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_types.lo -c u/t_game_types.cpp

u/t_game_types.o: u/t_game_types.cpp
	@echo "        Compiling u/t_game_types.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_types.o -c u/t_game_types.cpp

u/t_game_types.s: u/t_game_types.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_types.s -S u/t_game_types.cpp

u/t_game_unitscoredefinitionlist.lo: u/t_game_unitscoredefinitionlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscoredefinitionlist.lo -c u/t_game_unitscoredefinitionlist.cpp

u/t_game_unitscoredefinitionlist.o: u/t_game_unitscoredefinitionlist.cpp
	@echo "        Compiling u/t_game_unitscoredefinitionlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscoredefinitionlist.o -c u/t_game_unitscoredefinitionlist.cpp

u/t_game_unitscoredefinitionlist.s: u/t_game_unitscoredefinitionlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscoredefinitionlist.s -S u/t_game_unitscoredefinitionlist.cpp

u/t_game_unitscorelist.lo: u/t_game_unitscorelist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscorelist.lo -c u/t_game_unitscorelist.cpp

u/t_game_unitscorelist.o: u/t_game_unitscorelist.cpp
	@echo "        Compiling u/t_game_unitscorelist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscorelist.o -c u/t_game_unitscorelist.cpp

u/t_game_unitscorelist.s: u/t_game_unitscorelist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_unitscorelist.s -S u/t_game_unitscorelist.cpp

u/t_game_v3_resultfile.lo: u/t_game_v3_resultfile.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_resultfile.lo -c u/t_game_v3_resultfile.cpp

u/t_game_v3_resultfile.o: u/t_game_v3_resultfile.cpp
	@echo "        Compiling u/t_game_v3_resultfile.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_resultfile.o -c u/t_game_v3_resultfile.cpp

u/t_game_v3_resultfile.s: u/t_game_v3_resultfile.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_resultfile.s -S u/t_game_v3_resultfile.cpp

u/t_game_v3_stringverifier.lo: u/t_game_v3_stringverifier.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_stringverifier.lo -c u/t_game_v3_stringverifier.cpp

u/t_game_v3_stringverifier.o: u/t_game_v3_stringverifier.cpp
	@echo "        Compiling u/t_game_v3_stringverifier.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_stringverifier.o -c u/t_game_v3_stringverifier.cpp

u/t_game_v3_stringverifier.s: u/t_game_v3_stringverifier.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_v3_stringverifier.s -S u/t_game_v3_stringverifier.cpp

u/t_game_vcr_battle.lo: u/t_game_vcr_battle.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_battle.lo -c u/t_game_vcr_battle.cpp

u/t_game_vcr_battle.o: u/t_game_vcr_battle.cpp
	@echo "        Compiling u/t_game_vcr_battle.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_battle.o -c u/t_game_vcr_battle.cpp

u/t_game_vcr_battle.s: u/t_game_vcr_battle.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_battle.s -S u/t_game_vcr_battle.cpp

u/t_game_vcr_classic_algorithm.lo: u/t_game_vcr_classic_algorithm.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_algorithm.lo -c u/t_game_vcr_classic_algorithm.cpp

u/t_game_vcr_classic_algorithm.o: u/t_game_vcr_classic_algorithm.cpp
	@echo "        Compiling u/t_game_vcr_classic_algorithm.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_algorithm.o -c u/t_game_vcr_classic_algorithm.cpp

u/t_game_vcr_classic_algorithm.s: u/t_game_vcr_classic_algorithm.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_algorithm.s -S u/t_game_vcr_classic_algorithm.cpp

u/t_game_vcr_classic_hostalgorithm.lo: \
    u/t_game_vcr_classic_hostalgorithm.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_hostalgorithm.lo -c u/t_game_vcr_classic_hostalgorithm.cpp

u/t_game_vcr_classic_hostalgorithm.o: u/t_game_vcr_classic_hostalgorithm.cpp
	@echo "        Compiling u/t_game_vcr_classic_hostalgorithm.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_hostalgorithm.o -c u/t_game_vcr_classic_hostalgorithm.cpp

u/t_game_vcr_classic_hostalgorithm.s: u/t_game_vcr_classic_hostalgorithm.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_hostalgorithm.s -S u/t_game_vcr_classic_hostalgorithm.cpp

u/t_game_vcr_classic_nullvisualizer.lo: \
    u/t_game_vcr_classic_nullvisualizer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_nullvisualizer.lo -c u/t_game_vcr_classic_nullvisualizer.cpp

u/t_game_vcr_classic_nullvisualizer.o: \
    u/t_game_vcr_classic_nullvisualizer.cpp
	@echo "        Compiling u/t_game_vcr_classic_nullvisualizer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_nullvisualizer.o -c u/t_game_vcr_classic_nullvisualizer.cpp

u/t_game_vcr_classic_nullvisualizer.s: \
    u/t_game_vcr_classic_nullvisualizer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_nullvisualizer.s -S u/t_game_vcr_classic_nullvisualizer.cpp

u/t_game_vcr_classic_pvcralgorithm.lo: \
    u/t_game_vcr_classic_pvcralgorithm.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_pvcralgorithm.lo -c u/t_game_vcr_classic_pvcralgorithm.cpp

u/t_game_vcr_classic_pvcralgorithm.o: u/t_game_vcr_classic_pvcralgorithm.cpp
	@echo "        Compiling u/t_game_vcr_classic_pvcralgorithm.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_pvcralgorithm.o -c u/t_game_vcr_classic_pvcralgorithm.cpp

u/t_game_vcr_classic_pvcralgorithm.s: u/t_game_vcr_classic_pvcralgorithm.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_pvcralgorithm.s -S u/t_game_vcr_classic_pvcralgorithm.cpp

u/t_game_vcr_classic_statustoken.lo: u/t_game_vcr_classic_statustoken.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_statustoken.lo -c u/t_game_vcr_classic_statustoken.cpp

u/t_game_vcr_classic_statustoken.o: u/t_game_vcr_classic_statustoken.cpp
	@echo "        Compiling u/t_game_vcr_classic_statustoken.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_statustoken.o -c u/t_game_vcr_classic_statustoken.cpp

u/t_game_vcr_classic_statustoken.s: u/t_game_vcr_classic_statustoken.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_statustoken.s -S u/t_game_vcr_classic_statustoken.cpp

u/t_game_vcr_classic_visualizer.lo: u/t_game_vcr_classic_visualizer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_visualizer.lo -c u/t_game_vcr_classic_visualizer.cpp

u/t_game_vcr_classic_visualizer.o: u/t_game_vcr_classic_visualizer.cpp
	@echo "        Compiling u/t_game_vcr_classic_visualizer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_visualizer.o -c u/t_game_vcr_classic_visualizer.cpp

u/t_game_vcr_classic_visualizer.s: u/t_game_vcr_classic_visualizer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_classic_visualizer.s -S u/t_game_vcr_classic_visualizer.cpp

u/t_game_vcr_database.lo: u/t_game_vcr_database.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_database.lo -c u/t_game_vcr_database.cpp

u/t_game_vcr_database.o: u/t_game_vcr_database.cpp
	@echo "        Compiling u/t_game_vcr_database.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_database.o -c u/t_game_vcr_database.cpp

u/t_game_vcr_database.s: u/t_game_vcr_database.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_database.s -S u/t_game_vcr_database.cpp

u/t_game_vcr_object.lo: u/t_game_vcr_object.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_object.lo -c u/t_game_vcr_object.cpp

u/t_game_vcr_object.o: u/t_game_vcr_object.cpp
	@echo "        Compiling u/t_game_vcr_object.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_object.o -c u/t_game_vcr_object.cpp

u/t_game_vcr_object.s: u/t_game_vcr_object.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_object.s -S u/t_game_vcr_object.cpp

u/t_game_vcr_score.lo: u/t_game_vcr_score.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_score.lo -c u/t_game_vcr_score.cpp

u/t_game_vcr_score.o: u/t_game_vcr_score.cpp
	@echo "        Compiling u/t_game_vcr_score.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_score.o -c u/t_game_vcr_score.cpp

u/t_game_vcr_score.s: u/t_game_vcr_score.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_game_vcr_score.s -S u/t_game_vcr_score.cpp

u/t_gfx_canvas.lo: u/t_gfx_canvas.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_canvas.lo -c u/t_gfx_canvas.cpp

u/t_gfx_canvas.o: u/t_gfx_canvas.cpp
	@echo "        Compiling u/t_gfx_canvas.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_canvas.o -c u/t_gfx_canvas.cpp

u/t_gfx_canvas.s: u/t_gfx_canvas.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_canvas.s -S u/t_gfx_canvas.cpp

u/t_gfx_engine.lo: u/t_gfx_engine.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_engine.lo -c u/t_gfx_engine.cpp

u/t_gfx_engine.o: u/t_gfx_engine.cpp
	@echo "        Compiling u/t_gfx_engine.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_engine.o -c u/t_gfx_engine.cpp

u/t_gfx_engine.s: u/t_gfx_engine.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_engine.s -S u/t_gfx_engine.cpp

u/t_gfx_eventconsumer.lo: u/t_gfx_eventconsumer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_eventconsumer.lo -c u/t_gfx_eventconsumer.cpp

u/t_gfx_eventconsumer.o: u/t_gfx_eventconsumer.cpp
	@echo "        Compiling u/t_gfx_eventconsumer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_eventconsumer.o -c u/t_gfx_eventconsumer.cpp

u/t_gfx_eventconsumer.s: u/t_gfx_eventconsumer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_eventconsumer.s -S u/t_gfx_eventconsumer.cpp

u/t_gfx_fillpattern.lo: u/t_gfx_fillpattern.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_fillpattern.lo -c u/t_gfx_fillpattern.cpp

u/t_gfx_fillpattern.o: u/t_gfx_fillpattern.cpp
	@echo "        Compiling u/t_gfx_fillpattern.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_fillpattern.o -c u/t_gfx_fillpattern.cpp

u/t_gfx_fillpattern.s: u/t_gfx_fillpattern.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_fillpattern.s -S u/t_gfx_fillpattern.cpp

u/t_gfx_nullcanvas.lo: u/t_gfx_nullcanvas.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullcanvas.lo -c u/t_gfx_nullcanvas.cpp

u/t_gfx_nullcanvas.o: u/t_gfx_nullcanvas.cpp
	@echo "        Compiling u/t_gfx_nullcanvas.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullcanvas.o -c u/t_gfx_nullcanvas.cpp

u/t_gfx_nullcanvas.s: u/t_gfx_nullcanvas.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullcanvas.s -S u/t_gfx_nullcanvas.cpp

u/t_gfx_nullengine.lo: u/t_gfx_nullengine.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullengine.lo -c u/t_gfx_nullengine.cpp

u/t_gfx_nullengine.o: u/t_gfx_nullengine.cpp
	@echo "        Compiling u/t_gfx_nullengine.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullengine.o -c u/t_gfx_nullengine.cpp

u/t_gfx_nullengine.s: u/t_gfx_nullengine.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_nullengine.s -S u/t_gfx_nullengine.cpp

u/t_gfx_point.lo: u/t_gfx_point.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_point.lo -c u/t_gfx_point.cpp

u/t_gfx_point.o: u/t_gfx_point.cpp
	@echo "        Compiling u/t_gfx_point.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_point.o -c u/t_gfx_point.cpp

u/t_gfx_point.s: u/t_gfx_point.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_point.s -S u/t_gfx_point.cpp

u/t_gfx_rectangle.lo: u/t_gfx_rectangle.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_rectangle.lo -c u/t_gfx_rectangle.cpp

u/t_gfx_rectangle.o: u/t_gfx_rectangle.cpp
	@echo "        Compiling u/t_gfx_rectangle.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_rectangle.o -c u/t_gfx_rectangle.cpp

u/t_gfx_rectangle.s: u/t_gfx_rectangle.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_rectangle.s -S u/t_gfx_rectangle.cpp

u/t_gfx_resourceprovider.lo: u/t_gfx_resourceprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_resourceprovider.lo -c u/t_gfx_resourceprovider.cpp

u/t_gfx_resourceprovider.o: u/t_gfx_resourceprovider.cpp
	@echo "        Compiling u/t_gfx_resourceprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_resourceprovider.o -c u/t_gfx_resourceprovider.cpp

u/t_gfx_resourceprovider.s: u/t_gfx_resourceprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_resourceprovider.s -S u/t_gfx_resourceprovider.cpp

u/t_gfx_sdl_engine.lo: u/t_gfx_sdl_engine.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_sdl_engine.lo -c u/t_gfx_sdl_engine.cpp

u/t_gfx_sdl_engine.o: u/t_gfx_sdl_engine.cpp
	@echo "        Compiling u/t_gfx_sdl_engine.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_sdl_engine.o -c u/t_gfx_sdl_engine.cpp

u/t_gfx_sdl_engine.s: u/t_gfx_sdl_engine.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_sdl_engine.s -S u/t_gfx_sdl_engine.cpp

u/t_gfx_timer.lo: u/t_gfx_timer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timer.lo -c u/t_gfx_timer.cpp

u/t_gfx_timer.o: u/t_gfx_timer.cpp
	@echo "        Compiling u/t_gfx_timer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timer.o -c u/t_gfx_timer.cpp

u/t_gfx_timer.s: u/t_gfx_timer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timer.s -S u/t_gfx_timer.cpp

u/t_gfx_timerqueue.lo: u/t_gfx_timerqueue.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timerqueue.lo -c u/t_gfx_timerqueue.cpp

u/t_gfx_timerqueue.o: u/t_gfx_timerqueue.cpp
	@echo "        Compiling u/t_gfx_timerqueue.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timerqueue.o -c u/t_gfx_timerqueue.cpp

u/t_gfx_timerqueue.s: u/t_gfx_timerqueue.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_gfx_timerqueue.s -S u/t_gfx_timerqueue.cpp

u/t_interpreter.lo: u/t_interpreter.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter.lo -c u/t_interpreter.cpp

u/t_interpreter.o: u/t_interpreter.cpp
	@echo "        Compiling u/t_interpreter.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter.o -c u/t_interpreter.cpp

u/t_interpreter.s: u/t_interpreter.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter.s -S u/t_interpreter.cpp

u/t_interpreter_bytecodeobject.lo: u/t_interpreter_bytecodeobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_bytecodeobject.lo -c u/t_interpreter_bytecodeobject.cpp

u/t_interpreter_bytecodeobject.o: u/t_interpreter_bytecodeobject.cpp
	@echo "        Compiling u/t_interpreter_bytecodeobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_bytecodeobject.o -c u/t_interpreter_bytecodeobject.cpp

u/t_interpreter_bytecodeobject.s: u/t_interpreter_bytecodeobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_bytecodeobject.s -S u/t_interpreter_bytecodeobject.cpp

u/t_interpreter_closure.lo: u/t_interpreter_closure.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_closure.lo -c u/t_interpreter_closure.cpp

u/t_interpreter_closure.o: u/t_interpreter_closure.cpp
	@echo "        Compiling u/t_interpreter_closure.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_closure.o -c u/t_interpreter_closure.cpp

u/t_interpreter_closure.s: u/t_interpreter_closure.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_closure.s -S u/t_interpreter_closure.cpp

u/t_interpreter_exporter_fieldlist.lo: \
    u/t_interpreter_exporter_fieldlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_exporter_fieldlist.lo -c u/t_interpreter_exporter_fieldlist.cpp

u/t_interpreter_exporter_fieldlist.o: u/t_interpreter_exporter_fieldlist.cpp
	@echo "        Compiling u/t_interpreter_exporter_fieldlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_exporter_fieldlist.o -c u/t_interpreter_exporter_fieldlist.cpp

u/t_interpreter_exporter_fieldlist.s: u/t_interpreter_exporter_fieldlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_exporter_fieldlist.s -S u/t_interpreter_exporter_fieldlist.cpp

u/t_interpreter_expr_builtinfunction.lo: \
    u/t_interpreter_expr_builtinfunction.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_builtinfunction.lo -c u/t_interpreter_expr_builtinfunction.cpp

u/t_interpreter_expr_builtinfunction.o: \
    u/t_interpreter_expr_builtinfunction.cpp
	@echo "        Compiling u/t_interpreter_expr_builtinfunction.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_builtinfunction.o -c u/t_interpreter_expr_builtinfunction.cpp

u/t_interpreter_expr_builtinfunction.s: \
    u/t_interpreter_expr_builtinfunction.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_builtinfunction.s -S u/t_interpreter_expr_builtinfunction.cpp

u/t_interpreter_expr_parser.lo: u/t_interpreter_expr_parser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_parser.lo -c u/t_interpreter_expr_parser.cpp

u/t_interpreter_expr_parser.o: u/t_interpreter_expr_parser.cpp
	@echo "        Compiling u/t_interpreter_expr_parser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_parser.o -c u/t_interpreter_expr_parser.cpp

u/t_interpreter_expr_parser.s: u/t_interpreter_expr_parser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_expr_parser.s -S u/t_interpreter_expr_parser.cpp

u/t_interpreter_keywords.lo: u/t_interpreter_keywords.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_keywords.lo -c u/t_interpreter_keywords.cpp

u/t_interpreter_keywords.o: u/t_interpreter_keywords.cpp
	@echo "        Compiling u/t_interpreter_keywords.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_keywords.o -c u/t_interpreter_keywords.cpp

u/t_interpreter_keywords.s: u/t_interpreter_keywords.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_keywords.s -S u/t_interpreter_keywords.cpp

u/t_interpreter_nametable.lo: u/t_interpreter_nametable.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_nametable.lo -c u/t_interpreter_nametable.cpp

u/t_interpreter_nametable.o: u/t_interpreter_nametable.cpp
	@echo "        Compiling u/t_interpreter_nametable.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_nametable.o -c u/t_interpreter_nametable.cpp

u/t_interpreter_nametable.s: u/t_interpreter_nametable.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_nametable.s -S u/t_interpreter_nametable.cpp

u/t_interpreter_process.lo: u/t_interpreter_process.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_process.lo -c u/t_interpreter_process.cpp

u/t_interpreter_process.o: u/t_interpreter_process.cpp
	@echo "        Compiling u/t_interpreter_process.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_process.o -c u/t_interpreter_process.cpp

u/t_interpreter_process.s: u/t_interpreter_process.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_process.s -S u/t_interpreter_process.cpp

u/t_interpreter_processlist.lo: u/t_interpreter_processlist.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_processlist.lo -c u/t_interpreter_processlist.cpp

u/t_interpreter_processlist.o: u/t_interpreter_processlist.cpp
	@echo "        Compiling u/t_interpreter_processlist.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_processlist.o -c u/t_interpreter_processlist.cpp

u/t_interpreter_processlist.s: u/t_interpreter_processlist.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_processlist.s -S u/t_interpreter_processlist.cpp

u/t_interpreter_statementcompiler.lo: u/t_interpreter_statementcompiler.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_statementcompiler.lo -c u/t_interpreter_statementcompiler.cpp

u/t_interpreter_statementcompiler.o: u/t_interpreter_statementcompiler.cpp
	@echo "        Compiling u/t_interpreter_statementcompiler.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_statementcompiler.o -c u/t_interpreter_statementcompiler.cpp

u/t_interpreter_statementcompiler.s: u/t_interpreter_statementcompiler.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_statementcompiler.s -S u/t_interpreter_statementcompiler.cpp

u/t_interpreter_tokenizer.lo: u/t_interpreter_tokenizer.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_tokenizer.lo -c u/t_interpreter_tokenizer.cpp

u/t_interpreter_tokenizer.o: u/t_interpreter_tokenizer.cpp
	@echo "        Compiling u/t_interpreter_tokenizer.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_tokenizer.o -c u/t_interpreter_tokenizer.cpp

u/t_interpreter_tokenizer.s: u/t_interpreter_tokenizer.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_tokenizer.s -S u/t_interpreter_tokenizer.cpp

u/t_interpreter_values.lo: u/t_interpreter_values.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_values.lo -c u/t_interpreter_values.cpp

u/t_interpreter_values.o: u/t_interpreter_values.cpp
	@echo "        Compiling u/t_interpreter_values.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_values.o -c u/t_interpreter_values.cpp

u/t_interpreter_values.s: u/t_interpreter_values.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_interpreter_values.s -S u/t_interpreter_values.cpp

u/t_ui_colorscheme.lo: u/t_ui_colorscheme.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_colorscheme.lo -c u/t_ui_colorscheme.cpp

u/t_ui_colorscheme.o: u/t_ui_colorscheme.cpp
	@echo "        Compiling u/t_ui_colorscheme.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_colorscheme.o -c u/t_ui_colorscheme.cpp

u/t_ui_colorscheme.s: u/t_ui_colorscheme.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_colorscheme.s -S u/t_ui_colorscheme.cpp

u/t_ui_group.lo: u/t_ui_group.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_group.lo -c u/t_ui_group.cpp

u/t_ui_group.o: u/t_ui_group.cpp
	@echo "        Compiling u/t_ui_group.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_group.o -c u/t_ui_group.cpp

u/t_ui_group.s: u/t_ui_group.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_group.s -S u/t_ui_group.cpp

u/t_ui_invisiblewidget.lo: u/t_ui_invisiblewidget.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_invisiblewidget.lo -c u/t_ui_invisiblewidget.cpp

u/t_ui_invisiblewidget.o: u/t_ui_invisiblewidget.cpp
	@echo "        Compiling u/t_ui_invisiblewidget.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_invisiblewidget.o -c u/t_ui_invisiblewidget.cpp

u/t_ui_invisiblewidget.s: u/t_ui_invisiblewidget.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_invisiblewidget.s -S u/t_ui_invisiblewidget.cpp

u/t_ui_res_imageloader.lo: u/t_ui_res_imageloader.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_imageloader.lo -c u/t_ui_res_imageloader.cpp

u/t_ui_res_imageloader.o: u/t_ui_res_imageloader.cpp
	@echo "        Compiling u/t_ui_res_imageloader.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_imageloader.o -c u/t_ui_res_imageloader.cpp

u/t_ui_res_imageloader.s: u/t_ui_res_imageloader.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_imageloader.s -S u/t_ui_res_imageloader.cpp

u/t_ui_res_manager.lo: u/t_ui_res_manager.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_manager.lo -c u/t_ui_res_manager.cpp

u/t_ui_res_manager.o: u/t_ui_res_manager.cpp
	@echo "        Compiling u/t_ui_res_manager.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_manager.o -c u/t_ui_res_manager.cpp

u/t_ui_res_manager.s: u/t_ui_res_manager.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_manager.s -S u/t_ui_res_manager.cpp

u/t_ui_res_provider.lo: u/t_ui_res_provider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_provider.lo -c u/t_ui_res_provider.cpp

u/t_ui_res_provider.o: u/t_ui_res_provider.cpp
	@echo "        Compiling u/t_ui_res_provider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_provider.o -c u/t_ui_res_provider.cpp

u/t_ui_res_provider.s: u/t_ui_res_provider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_res_provider.s -S u/t_ui_res_provider.cpp

u/t_ui_rich_blockobject.lo: u/t_ui_rich_blockobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_blockobject.lo -c u/t_ui_rich_blockobject.cpp

u/t_ui_rich_blockobject.o: u/t_ui_rich_blockobject.cpp
	@echo "        Compiling u/t_ui_rich_blockobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_blockobject.o -c u/t_ui_rich_blockobject.cpp

u/t_ui_rich_blockobject.s: u/t_ui_rich_blockobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_blockobject.s -S u/t_ui_rich_blockobject.cpp

u/t_ui_rich_imageobject.lo: u/t_ui_rich_imageobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_imageobject.lo -c u/t_ui_rich_imageobject.cpp

u/t_ui_rich_imageobject.o: u/t_ui_rich_imageobject.cpp
	@echo "        Compiling u/t_ui_rich_imageobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_imageobject.o -c u/t_ui_rich_imageobject.cpp

u/t_ui_rich_imageobject.s: u/t_ui_rich_imageobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_rich_imageobject.s -S u/t_ui_rich_imageobject.cpp

u/t_ui_widget.lo: u/t_ui_widget.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widget.lo -c u/t_ui_widget.cpp

u/t_ui_widget.o: u/t_ui_widget.cpp
	@echo "        Compiling u/t_ui_widget.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widget.o -c u/t_ui_widget.cpp

u/t_ui_widget.s: u/t_ui_widget.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widget.s -S u/t_ui_widget.cpp

u/t_ui_widgets_focusiterator.lo: u/t_ui_widgets_focusiterator.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widgets_focusiterator.lo -c u/t_ui_widgets_focusiterator.cpp

u/t_ui_widgets_focusiterator.o: u/t_ui_widgets_focusiterator.cpp
	@echo "        Compiling u/t_ui_widgets_focusiterator.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widgets_focusiterator.o -c u/t_ui_widgets_focusiterator.cpp

u/t_ui_widgets_focusiterator.s: u/t_ui_widgets_focusiterator.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_ui_widgets_focusiterator.s -S u/t_ui_widgets_focusiterator.cpp

u/t_util_answerprovider.lo: u/t_util_answerprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_answerprovider.lo -c u/t_util_answerprovider.cpp

u/t_util_answerprovider.o: u/t_util_answerprovider.cpp
	@echo "        Compiling u/t_util_answerprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_answerprovider.o -c u/t_util_answerprovider.cpp

u/t_util_answerprovider.s: u/t_util_answerprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_answerprovider.s -S u/t_util_answerprovider.cpp

u/t_util_atomtable.lo: u/t_util_atomtable.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_atomtable.lo -c u/t_util_atomtable.cpp

u/t_util_atomtable.o: u/t_util_atomtable.cpp
	@echo "        Compiling u/t_util_atomtable.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_atomtable.o -c u/t_util_atomtable.cpp

u/t_util_atomtable.s: u/t_util_atomtable.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_atomtable.s -S u/t_util_atomtable.cpp

u/t_util_backupfile.lo: u/t_util_backupfile.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_backupfile.lo -c u/t_util_backupfile.cpp

u/t_util_backupfile.o: u/t_util_backupfile.cpp
	@echo "        Compiling u/t_util_backupfile.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_backupfile.o -c u/t_util_backupfile.cpp

u/t_util_backupfile.s: u/t_util_backupfile.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_backupfile.s -S u/t_util_backupfile.cpp

u/t_util_baseslaverequest.lo: u/t_util_baseslaverequest.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequest.lo -c u/t_util_baseslaverequest.cpp

u/t_util_baseslaverequest.o: u/t_util_baseslaverequest.cpp
	@echo "        Compiling u/t_util_baseslaverequest.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequest.o -c u/t_util_baseslaverequest.cpp

u/t_util_baseslaverequest.s: u/t_util_baseslaverequest.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequest.s -S u/t_util_baseslaverequest.cpp

u/t_util_baseslaverequestsender.lo: u/t_util_baseslaverequestsender.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequestsender.lo -c u/t_util_baseslaverequestsender.cpp

u/t_util_baseslaverequestsender.o: u/t_util_baseslaverequestsender.cpp
	@echo "        Compiling u/t_util_baseslaverequestsender.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequestsender.o -c u/t_util_baseslaverequestsender.cpp

u/t_util_baseslaverequestsender.s: u/t_util_baseslaverequestsender.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_baseslaverequestsender.s -S u/t_util_baseslaverequestsender.cpp

u/t_util_constantanswerprovider.lo: u/t_util_constantanswerprovider.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_constantanswerprovider.lo -c u/t_util_constantanswerprovider.cpp

u/t_util_constantanswerprovider.o: u/t_util_constantanswerprovider.cpp
	@echo "        Compiling u/t_util_constantanswerprovider.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_constantanswerprovider.o -c u/t_util_constantanswerprovider.cpp

u/t_util_constantanswerprovider.s: u/t_util_constantanswerprovider.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_constantanswerprovider.s -S u/t_util_constantanswerprovider.cpp

u/t_util_key.lo: u/t_util_key.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_key.lo -c u/t_util_key.cpp

u/t_util_key.o: u/t_util_key.cpp
	@echo "        Compiling u/t_util_key.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_key.o -c u/t_util_key.cpp

u/t_util_key.s: u/t_util_key.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_key.s -S u/t_util_key.cpp

u/t_util_keymap.lo: u/t_util_keymap.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymap.lo -c u/t_util_keymap.cpp

u/t_util_keymap.o: u/t_util_keymap.cpp
	@echo "        Compiling u/t_util_keymap.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymap.o -c u/t_util_keymap.cpp

u/t_util_keymap.s: u/t_util_keymap.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymap.s -S u/t_util_keymap.cpp

u/t_util_keymaptable.lo: u/t_util_keymaptable.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymaptable.lo -c u/t_util_keymaptable.cpp

u/t_util_keymaptable.o: u/t_util_keymaptable.cpp
	@echo "        Compiling u/t_util_keymaptable.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymaptable.o -c u/t_util_keymaptable.cpp

u/t_util_keymaptable.s: u/t_util_keymaptable.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keymaptable.s -S u/t_util_keymaptable.cpp

u/t_util_keystring.lo: u/t_util_keystring.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keystring.lo -c u/t_util_keystring.cpp

u/t_util_keystring.o: u/t_util_keystring.cpp
	@echo "        Compiling u/t_util_keystring.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keystring.o -c u/t_util_keystring.cpp

u/t_util_keystring.s: u/t_util_keystring.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_keystring.s -S u/t_util_keystring.cpp

u/t_util_math.lo: u/t_util_math.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_math.lo -c u/t_util_math.cpp

u/t_util_math.o: u/t_util_math.cpp
	@echo "        Compiling u/t_util_math.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_math.o -c u/t_util_math.cpp

u/t_util_math.s: u/t_util_math.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_math.s -S u/t_util_math.cpp

u/t_util_randomnumbergenerator.lo: u/t_util_randomnumbergenerator.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_randomnumbergenerator.lo -c u/t_util_randomnumbergenerator.cpp

u/t_util_randomnumbergenerator.o: u/t_util_randomnumbergenerator.cpp
	@echo "        Compiling u/t_util_randomnumbergenerator.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_randomnumbergenerator.o -c u/t_util_randomnumbergenerator.cpp

u/t_util_randomnumbergenerator.s: u/t_util_randomnumbergenerator.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_randomnumbergenerator.s -S u/t_util_randomnumbergenerator.cpp

u/t_util_request.lo: u/t_util_request.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_request.lo -c u/t_util_request.cpp

u/t_util_request.o: u/t_util_request.cpp
	@echo "        Compiling u/t_util_request.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_request.o -c u/t_util_request.cpp

u/t_util_request.s: u/t_util_request.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_request.s -S u/t_util_request.cpp

u/t_util_requestdispatcher.lo: u/t_util_requestdispatcher.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestdispatcher.lo -c u/t_util_requestdispatcher.cpp

u/t_util_requestdispatcher.o: u/t_util_requestdispatcher.cpp
	@echo "        Compiling u/t_util_requestdispatcher.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestdispatcher.o -c u/t_util_requestdispatcher.cpp

u/t_util_requestdispatcher.s: u/t_util_requestdispatcher.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestdispatcher.s -S u/t_util_requestdispatcher.cpp

u/t_util_requestreceiver.lo: u/t_util_requestreceiver.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestreceiver.lo -c u/t_util_requestreceiver.cpp

u/t_util_requestreceiver.o: u/t_util_requestreceiver.cpp
	@echo "        Compiling u/t_util_requestreceiver.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestreceiver.o -c u/t_util_requestreceiver.cpp

u/t_util_requestreceiver.s: u/t_util_requestreceiver.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestreceiver.s -S u/t_util_requestreceiver.cpp

u/t_util_requestthread.lo: u/t_util_requestthread.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestthread.lo -c u/t_util_requestthread.cpp

u/t_util_requestthread.o: u/t_util_requestthread.cpp
	@echo "        Compiling u/t_util_requestthread.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestthread.o -c u/t_util_requestthread.cpp

u/t_util_requestthread.s: u/t_util_requestthread.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_requestthread.s -S u/t_util_requestthread.cpp

u/t_util_rich_attribute.lo: u/t_util_rich_attribute.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_attribute.lo -c u/t_util_rich_attribute.cpp

u/t_util_rich_attribute.o: u/t_util_rich_attribute.cpp
	@echo "        Compiling u/t_util_rich_attribute.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_attribute.o -c u/t_util_rich_attribute.cpp

u/t_util_rich_attribute.s: u/t_util_rich_attribute.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_attribute.s -S u/t_util_rich_attribute.cpp

u/t_util_rich_colorattribute.lo: u/t_util_rich_colorattribute.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_colorattribute.lo -c u/t_util_rich_colorattribute.cpp

u/t_util_rich_colorattribute.o: u/t_util_rich_colorattribute.cpp
	@echo "        Compiling u/t_util_rich_colorattribute.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_colorattribute.o -c u/t_util_rich_colorattribute.cpp

u/t_util_rich_colorattribute.s: u/t_util_rich_colorattribute.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_colorattribute.s -S u/t_util_rich_colorattribute.cpp

u/t_util_rich_linkattribute.lo: u/t_util_rich_linkattribute.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_linkattribute.lo -c u/t_util_rich_linkattribute.cpp

u/t_util_rich_linkattribute.o: u/t_util_rich_linkattribute.cpp
	@echo "        Compiling u/t_util_rich_linkattribute.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_linkattribute.o -c u/t_util_rich_linkattribute.cpp

u/t_util_rich_linkattribute.s: u/t_util_rich_linkattribute.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_linkattribute.s -S u/t_util_rich_linkattribute.cpp

u/t_util_rich_parser.lo: u/t_util_rich_parser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_parser.lo -c u/t_util_rich_parser.cpp

u/t_util_rich_parser.o: u/t_util_rich_parser.cpp
	@echo "        Compiling u/t_util_rich_parser.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_parser.o -c u/t_util_rich_parser.cpp

u/t_util_rich_parser.s: u/t_util_rich_parser.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_parser.s -S u/t_util_rich_parser.cpp

u/t_util_rich_styleattribute.lo: u/t_util_rich_styleattribute.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_styleattribute.lo -c u/t_util_rich_styleattribute.cpp

u/t_util_rich_styleattribute.o: u/t_util_rich_styleattribute.cpp
	@echo "        Compiling u/t_util_rich_styleattribute.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_styleattribute.o -c u/t_util_rich_styleattribute.cpp

u/t_util_rich_styleattribute.s: u/t_util_rich_styleattribute.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_styleattribute.s -S u/t_util_rich_styleattribute.cpp

u/t_util_rich_text.lo: u/t_util_rich_text.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_text.lo -c u/t_util_rich_text.cpp

u/t_util_rich_text.o: u/t_util_rich_text.cpp
	@echo "        Compiling u/t_util_rich_text.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_text.o -c u/t_util_rich_text.cpp

u/t_util_rich_text.s: u/t_util_rich_text.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_text.s -S u/t_util_rich_text.cpp

u/t_util_rich_visitor.lo: u/t_util_rich_visitor.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_visitor.lo -c u/t_util_rich_visitor.cpp

u/t_util_rich_visitor.o: u/t_util_rich_visitor.cpp
	@echo "        Compiling u/t_util_rich_visitor.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_visitor.o -c u/t_util_rich_visitor.cpp

u/t_util_rich_visitor.s: u/t_util_rich_visitor.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_rich_visitor.s -S u/t_util_rich_visitor.cpp

u/t_util_skincolor.lo: u/t_util_skincolor.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_skincolor.lo -c u/t_util_skincolor.cpp

u/t_util_skincolor.o: u/t_util_skincolor.cpp
	@echo "        Compiling u/t_util_skincolor.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_skincolor.o -c u/t_util_skincolor.cpp

u/t_util_skincolor.s: u/t_util_skincolor.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_skincolor.s -S u/t_util_skincolor.cpp

u/t_util_slaveobject.lo: u/t_util_slaveobject.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaveobject.lo -c u/t_util_slaveobject.cpp

u/t_util_slaveobject.o: u/t_util_slaveobject.cpp
	@echo "        Compiling u/t_util_slaveobject.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaveobject.o -c u/t_util_slaveobject.cpp

u/t_util_slaveobject.s: u/t_util_slaveobject.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaveobject.s -S u/t_util_slaveobject.cpp

u/t_util_slaverequest.lo: u/t_util_slaverequest.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequest.lo -c u/t_util_slaverequest.cpp

u/t_util_slaverequest.o: u/t_util_slaverequest.cpp
	@echo "        Compiling u/t_util_slaverequest.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequest.o -c u/t_util_slaverequest.cpp

u/t_util_slaverequest.s: u/t_util_slaverequest.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequest.s -S u/t_util_slaverequest.cpp

u/t_util_slaverequestsender.lo: u/t_util_slaverequestsender.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequestsender.lo -c u/t_util_slaverequestsender.cpp

u/t_util_slaverequestsender.o: u/t_util_slaverequestsender.cpp
	@echo "        Compiling u/t_util_slaverequestsender.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequestsender.o -c u/t_util_slaverequestsender.cpp

u/t_util_slaverequestsender.s: u/t_util_slaverequestsender.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_slaverequestsender.s -S u/t_util_slaverequestsender.cpp

u/t_util_string.lo: u/t_util_string.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_string.lo -c u/t_util_string.cpp

u/t_util_string.o: u/t_util_string.cpp
	@echo "        Compiling u/t_util_string.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_string.o -c u/t_util_string.cpp

u/t_util_string.s: u/t_util_string.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_string.s -S u/t_util_string.cpp

u/t_util_unicodechars.lo: u/t_util_unicodechars.cpp
	$(CXX) -fPIC $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_unicodechars.lo -c u/t_util_unicodechars.cpp

u/t_util_unicodechars.o: u/t_util_unicodechars.cpp
	@echo "        Compiling u/t_util_unicodechars.cpp..."
	@$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_unicodechars.o -c u/t_util_unicodechars.cpp

u/t_util_unicodechars.s: u/t_util_unicodechars.cpp
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -D_CXXTEST_HAVE_EH -D_CXXTEST_HAVE_STD -g -o u/t_util_unicodechars.s -S u/t_util_unicodechars.cpp

ui/colorscheme1.lo: ui/colorscheme.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o ui/colorscheme1.lo -c ui/colorscheme.cpp

ui/colorscheme1.o: ui/colorscheme.cpp
	@echo "        Compiling ui/colorscheme.cpp..."
	@$(CXX) $(CXXFLAGS) -o ui/colorscheme1.o -c ui/colorscheme.cpp

ui/colorscheme1.s: ui/colorscheme.cpp
	$(CXX) $(CXXFLAGS) -o ui/colorscheme1.s -S ui/colorscheme.cpp

ui/root1.lo: ui/root.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o ui/root1.lo -c ui/root.cpp

ui/root1.o: ui/root.cpp
	@echo "        Compiling ui/root.cpp..."
	@$(CXX) $(CXXFLAGS) -o ui/root1.o -c ui/root.cpp

ui/root1.s: ui/root.cpp
	$(CXX) $(CXXFLAGS) -o ui/root1.s -S ui/root.cpp

ui/widget1.lo: ui/widget.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o ui/widget1.lo -c ui/widget.cpp

ui/widget1.o: ui/widget.cpp
	@echo "        Compiling ui/widget.cpp..."
	@$(CXX) $(CXXFLAGS) -o ui/widget1.o -c ui/widget.cpp

ui/widget1.s: ui/widget.cpp
	$(CXX) $(CXXFLAGS) -o ui/widget1.s -S ui/widget.cpp

ui/widgets/statictext1.lo: ui/widgets/statictext.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o ui/widgets/statictext1.lo -c ui/widgets/statictext.cpp

ui/widgets/statictext1.o: ui/widgets/statictext.cpp
	@echo "        Compiling ui/widgets/statictext.cpp..."
	@$(CXX) $(CXXFLAGS) -o ui/widgets/statictext1.o -c ui/widgets/statictext.cpp

ui/widgets/statictext1.s: ui/widgets/statictext.cpp
	$(CXX) $(CXXFLAGS) -o ui/widgets/statictext1.s -S ui/widgets/statictext.cpp

util/rich/parser1.lo: util/rich/parser.cpp
	$(CXX) -fPIC $(CXXFLAGS) -o util/rich/parser1.lo -c util/rich/parser.cpp

util/rich/parser1.o: util/rich/parser.cpp
	@echo "        Compiling util/rich/parser.cpp..."
	@$(CXX) $(CXXFLAGS) -o util/rich/parser1.o -c util/rich/parser.cpp

util/rich/parser1.s: util/rich/parser.cpp
	$(CXX) $(CXXFLAGS) -o util/rich/parser1.s -S util/rich/parser.cpp


include depend.mk

.cpp.lo:
	$(CXX) -fPIC $(CXXFLAGS) -o $@ -c $<

.cpp.o:
	@echo "        Compiling $<..."
	@$(CXX) $(CXXFLAGS) -o $@ -c $<

.cpp.s:
	$(CXX) $(CXXFLAGS) -o $@ -S $<


