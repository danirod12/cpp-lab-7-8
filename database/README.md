This folder is used as storage where three files will be automatically created

My IDE compiles and runs program in sub folder, so I use `./../database` for storage,
you may need to change it to `./database` or something other. Program would not start without
that folder, because on C++ 11 standard we cannot just create folder (Only platform-specific)

To read data in my database use any HEX editor like [that one](https://hexed.it)  
Each file contains data entries of fixed bytes size (different size for each file),
where first 4 bytes are definitely INT representing ID

<div style="text-align: center;">
transactions.d12 example:  
<img src="https://i.imgur.com/0iZhRdD.png" alt="Image"/>
</div>
