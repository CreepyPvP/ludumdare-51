-- vim.opt.errorformat = " %#%f(%l\\,%c):\\ %m"
vim.opt.makeprg = "make"
vim.keymap.set("n", "<A-c>", "<cmd>make PLATFORM=WIN SILENT=1<CR>");
vim.keymap.set("n", "<", "<cmd>tabnew term://run.bat<CR>");

vim.opt.grepprg = "rg --vimgrep"
vim.opt.grepformat = "%f:%l:%c:%m"

raylib_search_dir = "./raylib"

function search_raylib(opts)
    vim.cmd("silent grep -F " .. vim.fn.shellescape(opts.args) .. " " .. raylib_search_dir)
end
vim.api.nvim_create_user_command("Raylib", search_raylib, { nargs=1 })
