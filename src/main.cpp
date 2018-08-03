// #include "include/options.h"
#include "include/bmc.h"
#include "lib/utils/llvm_utils.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
// pragam'ed to aviod warnings due to llvm included files
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Transforms/Utils/Cloning.h"
#pragma GCC diagnostic pop

// void run_bmc( std::unique_ptr<llvm::Module>& module,
//               std::vector< comment >&,
//               options& o, z3::context& z3_ctx,
//               value_expr_map& m_,
//               std::map<llvm::Loop*, loopdata*>& ld_map,
//               name_map& lMap,
//               std::map<std::string, llvm::Value*>& evMap);

void prepare_module( options& o,
                     std::unique_ptr<llvm::Module>& module,
                     std::vector<comment>& comments,
                     // std::map<llvm::Value*, std::string>& lMap,
                     std::map< const bb*,
                     std::pair< std::vector<std::string>,std::vector<std::string> > >& block_comment_map
                     ) {
  llvm::legacy::PassManager passMan;
  passMan.add( llvm::createPromoteMemoryToRegisterPass() );
  passMan.add( llvm::createLoopRotatePass() ); // some params
  passMan.add( llvm::createAlwaysInlinerLegacyPass() );
  if( o.unwind && o.llvm_unroll ) {
    // Work around due to a bug in interface since LLVM 4.0 =======
    // setting unroll count via commmand line parsing
    std::string ustr = "-unroll-count=" + std::to_string(o.loop_unroll_count);
    setLLVMConfigViaCommandLineOptions( ustr );
    // ============================================================
    passMan.add( llvm::createLoopUnrollPass( 0, 100, o.loop_unroll_count ) );
  }
  if( o.dump_cfg ){
    //change current folder
    passMan.add( llvm::createCFGPrinterLegacyPassPass() );
  }
  passMan.run( *module.get() );

  estimate_comment_location( module, comments, block_comment_map );
}

void run_bmc( std::unique_ptr<llvm::Module>& module,
              std::vector<comment>& comments,
              options& o, z3::context& z3_ctx,
              value_expr_map& def_map_,
              // std::map<llvm::Loop*, loopdata*>& ld_map,
              name_map& lMap,
              std::map<std::string, llvm::Value*>& evMap)
{
  std::map<const bb*,
           std::pair<std::vector<std::string>,std::vector<std::string> > >
    bb_cmt_map;
  prepare_module( o, module, comments, bb_cmt_map);
  bmc b(module, bb_cmt_map, o, z3_ctx, def_map_,
        // ld_map,
        lMap, evMap);
  b.init_glb();
  b.run_bmc_pass();
  for( auto& it : b.func_formula_map ) {
    b.check_all_spec( it.second );
  }
}

int main(int argc, char** argv) {
  options o;
  boost::filesystem::path def_config("default.conf");
  if ( boost::filesystem::exists( def_config ) ) {
    o.parse_config(def_config);
  }

  if (!o.parse_cmdline(argc, argv)) return 0; // help was called

  z3::context z3_ctx;
  
  std::unique_ptr<llvm::Module> module;
  std::vector< comment > comments;

  // todo: why these maps here
  value_expr_map def_map(z3_ctx);
  name_map local_name_map;
  // std::map<llvm::Loop*, loopdata*> ld_map;
  std::map<std::string, llvm::Value*> expr_val_map;

  module = c2ir( o.filePath, o.globalContext, comments);

  if( o.verbosity > 8 ) {
    module->print( llvm::outs(), nullptr );
  }

  run_bmc( module, comments, o, z3_ctx, def_map,
           // ld_map,
           local_name_map,
           expr_val_map);

}
