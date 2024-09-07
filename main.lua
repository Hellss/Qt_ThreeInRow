Board = {}
Board.__index = Board


function Board:new()
    local self = setmetatable({}, Board)
    self:init()
    while (self:tick()) do end
    self:dump()
    return self
end


function Board:init()
    self.size = 10
    self.colors = {'A', 'B', 'C', 'D', 'E', 'F'}
    self.grid = {}
    for i = 1, self.size do
        self.grid[i] = {}
        for j = 1, self.size do
            self.grid[i][j] = self.colors[math.random(#self.colors)]
        end
    end
end


function Board:tick()
    local changed = false
    for i = 1, self.size do
        for j = 1, self.size do
            if self:checkAndRemove(i, j) then
                changed = true
            end
        end
    end

    if changed then
        self:collapse()
    end
    return changed
end


function Board:checkAndRemove(x, y, check)
    local color = self.grid[x][y]
    local matches = {}
    
    -- Check horizontal
    local x0 = x
    local count = 1
    for i = x + 1, self.size do
        if self.grid[i][y] == color then
            count = count + 1
        else
            break
        end
    end
    for i = x - 1, 1, -1 do
        if self.grid[i][y] == color then
            count = count + 1
            x0 = i
        else
            break
        end
    end
    if count >= 3 then
        for i = x0, x0 + count - 1 do
            table.insert(matches, {i, y})
        end
    end

    -- Check vertical
    local y0 = y
    count = 1
    for j = y + 1, self.size do
        if self.grid[x][j] == color then
            count = count + 1
        else
            break
        end
    end
    for j = y - 1, 1, -1 do
        if self.grid[x][j] == color then
            count = count + 1
            y0 = j
        else
            break
        end
    end
    if count >= 3 then
        for j = y0, y0 + count - 1 do
            table.insert(matches, {x, j})
        end
    end

    if #matches > 0 then
        if not check then
            for _, pos in ipairs(matches) do
                self.grid[pos[1]][pos[2]] = nil
            end
        end
        return true
    end
    return false
end


function Board:collapse()
    for j = 1, self.size do
        local emptyCells = 0

        for i = self.size, 1, -1 do
            if self.grid[i][j] == nil then
                emptyCells = emptyCells + 1
            elseif emptyCells > 0 then
                self.grid[i + emptyCells][j] = self.grid[i][j]
                self.grid[i][j] = nil
            end
        end

        for i = 1, emptyCells do
            self.grid[i][j] = self.colors[math.random(#self.colors)]
        end
    end
end


function Board:mix()
    local flattened = {}
    for i = 1, self.size do
        for j = 1, self.size do
            table.insert(flattened, self.grid[i][j])
        end
    end

    for i = #flattened, 2, -1 do
        local j = math.random(i)
        flattened[i], flattened[j] = flattened[j], flattened[i]
    end

    for i = 1, self.size do
        for j = 1, self.size do
            self.grid[i][j] = flattened[(i - 1) * self.size + j]
        end
    end
end

function Board:move(x, y, dir)
    local dx, dy = 0, 0
    if dir == 'l' then dx = -1
    elseif dir == 'r' then dx = 1
    elseif dir == 'u' then dy = -1
    elseif dir == 'd' then dy = 1
    else return false end

    local nx, ny = x + dx, y + dy
    if nx < 1 or nx > self.size or ny < 1 or ny > self.size then
        return false
    end
    self.grid[y][x], self.grid[ny][nx] = self.grid[ny][nx], self.grid[y][x]
    return true
end

function Board:dump()
    print("    0 1 2 3 4 5 6 7 8 9")
    print("    -------------------")
    for i = 1, self.size do
        io.write(i-1 .. " | ")
        for j = 1, self.size do
            io.write(self.grid[i][j] .. " ")
        end
        print()
    end
end


function Board:hasPossibleMoves()
    local directions = {'l', 'r', 'u', 'd'}
    for x = 1, self.size do
        for y = 1, self.size do
            for _, dir in ipairs(directions) do
                local dx, dy = 0, 0
                if dir == 'l' then dx = -1
                elseif dir == 'r' then dx = 1
                elseif dir == 'u' then dy = -1
                elseif dir == 'd' then dy = 1 end

                local nx, ny = x + dx, y + dy
                if nx >= 1 and nx <= self.size and ny >= 1 and ny <= self.size then
                    -- Swap and check
                    self.grid[y][x], self.grid[ny][nx] = self.grid[ny][nx], self.grid[y][x]
                    if self:checkAndRemove(x, y, true) or self:checkAndRemove(nx, ny, true) then
                        -- Swap back and return true
                        self.grid[y][x], self.grid[ny][nx] = self.grid[ny][nx], self.grid[y][x]
                        return true
                    end
                    -- Swap back
                    self.grid[y][x], self.grid[ny][nx] = self.grid[ny][nx], self.grid[y][x]
                end
            end
        end
    end
    return false
end


board = Board:new()

while true do
    local mixed = false
    while not board:hasPossibleMoves() do
        print("No possible moves left, mixing the board.")
        board:mix()
        board:dump()
        mixed = true
    end
    if mixed then
        while (board:tick()) do
            board:dump()
        end
    end

    io.write("> ")
    local input = io.read()
    if not input or input == 'q' then
        break
    end
    
    local cmd, x, y, dir = string.match(input, "(%a) (%d) (%d) (%a)")
	if not tonumber(x) or not tonumber(y) then
		print("Usage: m x y dir(l - left, r - right, u - up, d - down)")
		print("Example: m 0 0 r")
	else
		x, y = tonumber(x) + 1, tonumber(y) + 1 -- convert to lua indexes

		if cmd == 'm' and board:move(x, y, dir) then
			board:dump()
			local changed = board:tick()
			if changed then
				board:dump()
				while (board:tick()) do
					board:dump()
				end
			else
				print("No combinations - revert operation moving")
				board:move(x, y, dir)
				board:dump()
			end
		else
			print("Invalid command or move.")
		end
	end
end
