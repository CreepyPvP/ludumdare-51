-- vim.opt.errorformat = " %#%f(%l\\,%c):\\ %m"
vim.opt.makeprg = "make"
vim.keymap.set("n", "<A-c>", "<cmd>make PLATFORM=WIN SILENT=1<CR>");
vim.keymap.set("n", "<", "<cmd>tabnew term://run.bat<CR>");
