USE [master]
GO
/****** Object:  Database [%DB_NAME%]    Script Date: 08/22/2007 15:43:16 ******/
CREATE DATABASE [%DB_NAME%] ON  PRIMARY 
( NAME = N'%DB_NAME%', FILENAME = N'%DB_PATH%\%DB_NAME%.MDF' , SIZE = 5120KB , MAXSIZE = UNLIMITED, FILEGROWTH = 10%)
 LOG ON 
( NAME = N'%DB_NAME%_Log', FILENAME = N'%DB_PATH%\%DB_NAME%.LDF' , SIZE = 1024KB , MAXSIZE = UNLIMITED , FILEGROWTH = 10%)
GO
EXEC dbo.sp_dbcmptlevel @dbname=N'%DB_NAME%', @new_cmptlevel=80
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [%DB_NAME%].[dbo].[sp_fulltext_database] @action = 'disable'
end
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [%DB_NAME%] SET ARITHABORT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_CLOSE ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_CREATE_STATISTICS ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_SHRINK ON 
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [%DB_NAME%] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [%DB_NAME%] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [%DB_NAME%] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [%DB_NAME%] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [%DB_NAME%] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [%DB_NAME%] SET  MULTI_USER 

USE [%DB_NAME%]
GO
--
-- Generate script from macros replacing macro-parameters by real values passed into.
--
CREATE FUNCTION [dbo].[fn_GenSQL] (@Script nvarchar(4000), @Parameters nvarchar(4000))
RETURNS nvarchar(4000) 
AS  BEGIN 
    DECLARE @position int
    DECLARE @param nvarchar(4000)
    DECLARE @from nvarchar(4000)
    DECLARE @to nvarchar(4000)
    -- We use '|' as devider for parameters
    SET @Parameters = @Parameters + '|'
    
    SET @position = CHARINDEX( '|', @Parameters )
    WHILE ( @position <> 0 ) BEGIN
            SET @param  = LEFT( @Parameters, @position - 1 )
            SET @Parameters = RIGHT( @Parameters, LEN( @Parameters ) - @position )
            SET @position = CHARINDEX( '=', @param)
            IF ( @position = 0 ) BREAK
            SET @from = LEFT( @param, @position - 1 )
            SET @from = REPLACE( @from, ' ', '' )
            SET @to = RIGHT( @param, LEN( @param ) - @position )

            SET @Script = REPLACE( @Script, @from, @to )
            SET @position = CHARINDEX( '|', @Parameters )
    END
    RETURN @Script
END
GO
/****** Object:  StoredProcedure [dbo].[sp_GetObjectProperies]    Script Date: 08/22/2007 15:25:13 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
--
-- Retrives all property names of object
--
CREATE PROCEDURE [dbo].[sp_GetObjectProperies] ( @objectID int )
AS
    DECLARE @errorMessage nvarchar(255)
    SET NOCOUNT ON
    -- Check input parameters
    IF ( @objectID = NULL ) BEGIN
            SET @errorMessage = 'Incorect input values'
            GOTO err_
    END
    EXECUTE (  ' SELECT [Name]
                          FROM _properties
                 WHERE [ObjectID] =  ' + @objectID +
                          '    UNION
                          SELECT [Name]
                          FROM _images 
                 WHERE [ObjectID] = ' + @objectID )
 
    RETURN
err_:
    RAISERROR( @errorMessage, 11, 1 )
GO
/****** Object:  Table [dbo].[_objects]    Script Date: 08/22/2007 15:25:23 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[_objects](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectName] [nvarchar](255) NOT NULL,
	[ObjectType] [nvarchar](255) NOT NULL,
	[TimeStamp] [datetime] NULL,
 CONSTRAINT [PK_Objects] PRIMARY KEY CLUSTERED 
(
	[ID] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
CREATE NONCLUSTERED INDEX [IX_Objects_ObjectType] ON [dbo].[_objects] 
(
	[ObjectType] ASC
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_images]    Script Date: 08/22/2007 15:25:17 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[_images](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectID] [int] NOT NULL,
	[Name] [nvarchar](50) NOT NULL,
	[Value] [image] NULL,
 CONSTRAINT [PK__images] PRIMARY KEY CLUSTERED 
(
	[ObjectID] ASC,
	[Name] ASC
) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_links]    Script Date: 08/22/2007 15:25:20 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[_links](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[Parent] [int] NOT NULL,
	[Child] [int] NOT NULL,
 CONSTRAINT [PK__links] PRIMARY KEY CLUSTERED 
(
	[Parent] ASC,
	[Child] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Table [dbo].[_properties]    Script Date: 08/22/2007 15:25:26 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[_properties](
	[ID] [int] IDENTITY(1,1) NOT NULL,
	[ObjectID] [int] NOT NULL,
	[Name] [nvarchar](50) NOT NULL,
	[Value] [sql_variant] NULL,
 CONSTRAINT [PK_Properties] PRIMARY KEY CLUSTERED 
(
	[ObjectID] ASC,
	[Name] ASC
) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  StoredProcedure [dbo].[sp_GetObjectsByCriteria]    Script Date: 08/22/2007 15:25:13 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
--
-- Search for objects by given WHERE request (called by server)
--
CREATE PROCEDURE [dbo].[sp_GetObjectsByCriteria]
                                      @Request nvarchar(4000), @View nvarchar(4000), @First int, @Count int, @OrderBy nvarchar(4000)
AS
    DECLARE @sql nvarchar(4000)
    DECLARE @params nvarchar(4000)
    DECLARE @errorMessage nvarchar(255)
    --variables for OrderBy build routines
    DECLARE @_cont bit SET @_cont = 1
    DECLARE @_first int SET @_first = 1
    DECLARE @_pos int
    DECLARE @_prop as nvarchar(50)
    DECLARE @_join nvarchar(2000) SET @_join = ''
    DECLARE @_orderBy nvarchar(255) SET @_orderBy = ''
    DECLARE @_uprop nvarchar(100) 
    DECLARE @_specifier nvarchar(4)
    DECLARE @_sortcol nvarchar(50)
    --end
    -- Not display messages
    SET NOCOUNT ON
    -- If view is empty then get standart view
    IF ( ( LEN( @Request ) = 0 ) OR ( @Request = NULL) ) SET @Request = '0=0'
    IF ( ( LEN( @View ) = 0 )  OR ( @View = NULL )) BEGIN
            SET @errorMessage = 'Incorect input values'
            GOTO err_
    END
    IF ( ( @First = 0 ) OR ( @First = NULL )) SET @First = 1
    IF ( ( @Count = 0 ) OR ( @Count = NULL ) ) SET @Count = 2147483647
    IF ( ( LEN( @OrderBy ) = 0 ) OR ( @OrderBy = NULL ) )  SET @_orderBy = 'NULL'
    ELSE BEGIN
        --build OrderBy
        WHILE ( @_cont = 1 ) BEGIN
            -- find end of sort rule next to @_first position
            SET @_pos = CHARINDEX ( ',', @OrderBy, @_first ) 
            --sort rule isn't last
            IF ( @_pos <> 0 ) BEGIN
                --get separated sort rule
                SET @_prop = RTRIM(LTRIM(SUBSTRING( @OrderBy, @_first, @_pos  - @_first )))
                --save position to begin search from for next iteration
                SET @_first = @_pos + 1
            END
            --sort rule is last
            ELSE BEGIN
                --get separated sort rule
                SET @_prop = RTRIM(LTRIM(SUBSTRING( @OrderBy, @_first, LEN(@OrderBy) - @_first + 1)))
                --disable next iteration
                SET @_cont = 0
            END 
            
            --converted to uppercase property
            SET @_uprop = UPPER(@_prop)
            -- check for ascending sorting order modifier
            SET @_pos = PATINDEX ( '%ASC', @_uprop ) 
            IF ( @_pos > 0 ) BEGIN
                SET @_specifier = SUBSTRING(@_uprop, @_pos, 3)
                SET @_sortcol = LTRIM(RTRIM(SUBSTRING(@_prop, 1,  @_pos - 1)))
            END
            -- check for descending sorting order modifier
            ELSE BEGIN
                SET @_pos = PATINDEX( '%DESC', @_uprop )
                    
                IF (@_pos > 0 ) BEGIN
                    SET @_specifier = SUBSTRING(@_uprop, @_pos, 4)
                    SET @_sortcol = LTRIM(RTRIM(SUBSTRING(@_prop, 1, @_pos - 1)))
                END
                ELSE BEGIN
                    SET @_sortcol  = @_prop
                END
                -- append 
                SET @_join = @_join  + ' LEFT JOIN _properties as _' + @_sortcol + ' ON Source.[id] = _' + @_sortcol + '.ObjectID AND _' + @_sortcol +'.[name] = ''' + @_sortcol + ''''
                SET @_orderBy = @_orderBy + '_' + @_sortcol + '.[Value]'
                
                IF (LEN(@_specifier) > 0 )  SET @_orderBy = @_orderBy + ' ' + @_specifier + ' '
                SET @_specifier = NULL
                SET  @_orderBy  = @_orderBy + ','
            END
        END
       
        IF ( LEN(@_orderBy) > 0  ) SET @_orderBy = SUBSTRING(@_orderBy, 0, LEN(@_orderBy) )
        --end
    END
    /*Declare macros
     @c_View		- view for output (if = '' then use standart)
     @c_Request		- must be true expression (for example '0=0') or select without WHERE
     @c_First		- first number of returned row
     @c_Count		- number of rows to return
     @c_OrderBy		- order by instruction
    */
    SET @sql = '
            DECLARE @counter as int
            DECLARE @id as int
            DECLARE @resultID TABLE ([ids] int)
            -- Check for user rights and return requestet count of items
            DECLARE cursor_Found CURSOR SCROLL FOR 
            SELECT Source.[ID] FROM @c_View AS Source @c_Join WHERE @c_Request ORDER BY @c_OrderBy
            OPEN cursor_Found
                    SET @counter = 0
                    FETCH ABSOLUTE @c_First FROM cursor_Found INTO @id
                    WHILE( (@@FETCH_STATUS = 0) AND (@counter < @c_Count) ) BEGIN
                            INSERT INTO @resultID ([ids]) VALUES (@id)
                            FETCH NEXT FROM cursor_Found INTO @id
                            SET @counter = @counter + 1
                    END
            CLOSE cursor_Found
            DEALLOCATE cursor_Found
            -- Make SQL request
            SELECT [ID], [ObjectName], [ObjectType], [TimeStamp] FROM _objects INNER JOIN @resultID as resultID ON _objects.ID = resultID.ids'
    -- Set parameters to change in macros
    SET @params = '@c_Request=' + @Request +
                               '|@c_Join=' + @_join +
                               '|@c_View=' + @View +
                               '|@c_First=' + cast(@First as nvarchar(4000) ) + 
                               '|@c_Count=' + cast(@Count as nvarchar(4000)) +
                               '|@c_OrderBy=' + @_orderBy
    -- Change macros parameters
    SET @sql = dbo.fn_GenSQL( @sql, @params )
    -- Execute macros
    EXECUTE( @sql )
exit_:
    RETURN
err_:
    RAISERROR( @errorMessage, 11, 1 )
GO
/****** Object:  Trigger [objects]    Script Date: 08/22/2007 15:25:28 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[objects] ON [dbo].[_objects] FOR INSERT, UPDATE
AS
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN (SELECT [ID] FROM [inserted])
GO
/****** Object:  Trigger [images_objects]    Script Date: 08/22/2007 15:25:27 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[images_objects] ON [dbo].[_images] FOR INSERT, UPDATE, DELETE
AS
    -- check for unique property names
    IF ( EXISTS(SELECT *
        FROM (
            SELECT [ObjectID], [Name] FROM [_properties]
            UNION
            SELECT [ObjectID], [Name] FROM [_images]) p
            GROUP BY
            [ObjectID], [Name]
            HAVING
            COUNT(*) > 1) ) BEGIN
        RAISERROR ('There is property unique naming conflict!', 11, 1)
        ROLLBACK TRANSACTION
    END
    -- update object timestamp
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN ((SELECT [ObjectID] FROM [deleted]) UNION (SELECT [ObjectID] FROM [inserted]))
GO
/****** Object:  StoredProcedure [dbo].[sp_DeleteObject]    Script Date: 08/22/2007 15:25:12 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
--
-- Deletes object and it's properties
--
CREATE PROCEDURE [dbo].[sp_DeleteObject] ( @objectID int )
AS
    DECLARE @errorMessage nvarchar(255)
    SET NOCOUNT ON
    -- Check input parameters
    IF ( @objectID = NULL ) BEGIN
            SET @errorMessage = 'Incorect input values'
            GOTO err_
    END
    -- delete all properties of object and then object.
    DELETE FROM _links WHERE ([Parent] =  @objectID )
    DELETE FROM _properties WHERE [ObjectID] = @objectID
    DELETE FROM _images WHERE [ObjectID] = @objectID
    DELETE FROM _objects WHERE [ID] = @objectID
    RETURN
err_:
    RAISERROR( @errorMessage, 11, 1 )
GO
/****** Object:  UserDefinedFunction [dbo].[fn_GetObjectProps]    Script Date: 08/22/2007 15:25:27 ******/
SET ANSI_NULLS OFF
GO
SET QUOTED_IDENTIFIER OFF
GO
CREATE FUNCTION [dbo].[fn_GetObjectProps]
		(@ObjectID int)
RETURNS TABLE
RETURN 
            ( SELECT [Name]
              FROM _properties
              WHERE ObjectID = @ObjectID
                  UNION
             SELECT [Name]
             FROM _images
             WHERE ObjectID = @ObjectID)
GO
/****** Object:  Trigger [links_objects]    Script Date: 08/22/2007 15:25:28 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[links_objects] ON [dbo].[_links] FOR INSERT, UPDATE, DELETE
AS
  -- update object timestamp
  UPDATE [_objects] SET [TimeStamp] = GETDATE()
  WHERE [ID] IN (SELECT [Parent] FROM [deleted]
                 UNION
                 SELECT [Parent] FROM [inserted])
GO

/****** Object:  Trigger [properties_objects]    Script Date: 08/22/2007 15:25:28 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/* Template: Update object modification time */
CREATE TRIGGER [dbo].[properties_objects] ON [dbo].[_properties] FOR INSERT, UPDATE, DELETE
AS
    -- check for unique property names
    IF ( EXISTS(SELECT *
        FROM ( SELECT [ObjectID], [Name] FROM [_properties]
            UNION
            SELECT [ObjectID], [Name] FROM [_images]) p
            GROUP BY
            [ObjectID], [Name]
            HAVING
            COUNT(*) > 1) ) BEGIN
        RAISERROR ('There is property unique naming conflict!', 11, 1)
        ROLLBACK TRANSACTION
    END

    -- update object timestamp
    UPDATE [_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN ((SELECT [ObjectID] FROM [deleted]) UNION (SELECT [ObjectID] FROM [inserted]))
GO
/****** Object:  ForeignKey [FK_images_objects]    Script Date: 08/22/2007 15:25:17 ******/
ALTER TABLE [dbo].[_images]  WITH NOCHECK ADD  CONSTRAINT [FK_images_objects] FOREIGN KEY([ObjectID])
REFERENCES [dbo].[_objects] ([ID])
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_images] CHECK CONSTRAINT [FK_images_objects]
GO
/****** Object:  ForeignKey [FK_links_objects_1]    Script Date: 08/22/2007 15:25:20 ******/
ALTER TABLE [dbo].[_links]  WITH NOCHECK ADD  CONSTRAINT [FK_links_Parent] FOREIGN KEY([Parent])
REFERENCES [dbo].[_objects] ([ID])
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_links] CHECK CONSTRAINT [FK_links_Parent]
GO
/****** Object:  ForeignKey [FK_links_objects_2]    Script Date: 08/22/2007 15:25:20 ******/
ALTER TABLE [dbo].[_links]  WITH NOCHECK ADD  CONSTRAINT [FK_links_Child] FOREIGN KEY([Child])
REFERENCES [dbo].[_objects] ([ID])
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_links] CHECK CONSTRAINT [FK_links_Child]
GO
/****** Object:  ForeignKey [FK_properties_objects]    Script Date: 08/22/2007 15:25:26 ******/
ALTER TABLE [dbo].[_properties]  WITH NOCHECK ADD  CONSTRAINT [FK_properties_objects] FOREIGN KEY([ObjectID])
REFERENCES [dbo].[_objects] ([ID])
NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_properties] CHECK CONSTRAINT [FK_properties_objects]
GO