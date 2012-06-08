USE [master]
GO


/******************************************************************************/
/*                               CREATE DATABASE                              */
/******************************************************************************/
CREATE DATABASE [%DB_NAME%] ON PRIMARY
 ( NAME = N'%DB_NAME%', FILENAME = N'%DB_PATH%\%DB_NAME%.MDF', SIZE = 5120KB, MAXSIZE = UNLIMITED, FILEGROWTH = 10%)
 LOG ON
 ( NAME = N'%DB_NAME%_Log', FILENAME = N'%DB_PATH%\%DB_NAME%.LDF', SIZE = 1024KB, MAXSIZE = UNLIMITED, FILEGROWTH = 10%)
GO
ALTER DATABASE [%DB_NAME%] SET RECOVERY SIMPLE
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_SHRINK ON
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_PADDING ON
GO
ALTER DATABASE [%DB_NAME%] SET ANSI_WARNINGS ON
GO
ALTER DATABASE [%DB_NAME%] SET ARITHABORT ON
GO
ALTER DATABASE [%DB_NAME%] SET CONCAT_NULL_YIELDS_NULL ON
GO
ALTER DATABASE [%DB_NAME%] SET NUMERIC_ROUNDABORT OFF
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_CREATE_STATISTICS ON
GO
ALTER DATABASE [%DB_NAME%] SET AUTO_UPDATE_STATISTICS ON
GO
ALTER DATABASE [%DB_NAME%] SET MULTI_USER
GO
ALTER DATABASE [%DB_NAME%] SET READ_COMMITTED_SNAPSHOT ON
GO
USE [%DB_NAME%]
GO


/******************************************************************************/
/*                           Toolkit.RPL.Storage.ODB                          */
/******************************************************************************/

/****************************** [dbo].[_objects] ******************************/
CREATE TABLE [dbo].[_objects](
    [ID] int IDENTITY(1,1) NOT NULL,
    [ObjectName] nvarchar(255) NOT NULL,
    [ObjectType] nvarchar(195) NOT NULL,
    [TimeStamp] datetime NOT NULL DEFAULT GETDATE(),
  CONSTRAINT [PK_objects] PRIMARY KEY CLUSTERED ([ID] ASC)
)
CREATE NONCLUSTERED INDEX [IX_objects] ON [dbo].[_objects]
(
    [ObjectType] ASC
)
GO

CREATE TRIGGER [objects] ON [dbo].[_objects]
       FOR UPDATE
AS
    UPDATE [dbo].[_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN( SELECT [ID] FROM [inserted] )
GO

/******************************* [dbo].[_links] *******************************/
CREATE TABLE [dbo].[_links](
    [ID] int IDENTITY(1,1) NOT NULL,
    [Parent] int NOT NULL,
    [Child] int NOT NULL,
  CONSTRAINT [PK_links] PRIMARY KEY CLUSTERED ([ID] ASC)
)
CREATE UNIQUE NONCLUSTERED INDEX [IX_links] ON [dbo].[_links]
(
    [Parent] ASC,
    [Child] ASC
)
GO

ALTER TABLE [dbo].[_links] ADD
    CONSTRAINT [FK_links_Parent]
        FOREIGN KEY ([Parent]) REFERENCES [dbo].[_objects] ([ID])
    NOT FOR REPLICATION
GO
ALTER TABLE [dbo].[_links] ADD
    CONSTRAINT [FK_links_Child]
        FOREIGN KEY ([Child]) REFERENCES [dbo].[_objects] ([ID])
    ON DELETE CASCADE
    NOT FOR REPLICATION
GO

CREATE TRIGGER [links_objects] ON [dbo].[_links]
       FOR INSERT, UPDATE, DELETE
AS
    -- update object timestamp
    UPDATE [dbo].[_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN( SELECT [Parent] FROM [deleted]
                   UNION
                   SELECT [Parent] FROM [inserted] )
GO

/**************************** [dbo].[_properties] *****************************/
CREATE TABLE [dbo].[_properties](
    [ID] int IDENTITY(1,1) NOT NULL,
    [ObjectID] int NOT NULL,
    [Name] nvarchar(50) NOT NULL,
    [Value] sql_variant NULL,
  CONSTRAINT [PK_properties] PRIMARY KEY CLUSTERED ([ID] ASC)
)
CREATE UNIQUE NONCLUSTERED INDEX [IX_properties] ON [dbo].[_properties]
(
    [ObjectID] ASC,
    [Name] ASC
)
GO

ALTER TABLE [dbo].[_properties] ADD
    CONSTRAINT [FK_properties_objects]
        FOREIGN KEY ([ObjectID]) REFERENCES [dbo].[_objects] ([ID])
    ON DELETE CASCADE
    NOT FOR REPLICATION
GO

CREATE TRIGGER [properties_objects] ON [dbo].[_properties]
       FOR INSERT, UPDATE, DELETE
AS
    -- update object timestamp
    UPDATE [dbo].[_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN( SELECT [ObjectID] FROM [deleted]
                   UNION
                   SELECT [ObjectID] FROM [inserted] )
GO

CREATE TRIGGER [properties_images] ON [dbo].[_properties]
       FOR INSERT, UPDATE
AS
    -- check for unique property names
    IF( EXISTS( SELECT *
                FROM   [inserted] [i]
                       JOIN
                       [dbo].[_images] [img] ON
                           [i].[ObjectID] = [img].[ObjectID] AND
                           [i].[Name] = [img].[Name] ) )
    BEGIN
        RAISERROR( 'There is property unique naming conflict!', 11, 1 )
        ROLLBACK TRANSACTION
    END
GO

/****************************** [dbo].[_images] *******************************/
CREATE TABLE [dbo].[_images](
    [ID] int IDENTITY(1,1) NOT NULL,
    [ObjectID] int NOT NULL,
    [Name] nvarchar(50) NOT NULL,
    [Value] image NULL,
  CONSTRAINT [PK_images] PRIMARY KEY CLUSTERED ([ID] ASC)
)
CREATE UNIQUE NONCLUSTERED INDEX [IX_images] ON [dbo].[_images]
(
    [ObjectID] ASC,
    [Name] ASC
)
GO

ALTER TABLE [dbo].[_images] ADD
    CONSTRAINT [FK_images_objects]
        FOREIGN KEY ([ObjectID]) REFERENCES [dbo].[_objects] ([ID])
    ON DELETE CASCADE
    NOT FOR REPLICATION
GO

CREATE TRIGGER [images_objects] ON [dbo].[_images]
       FOR INSERT, UPDATE, DELETE
AS
    -- update object timestamp
    UPDATE [dbo].[_objects] SET [TimeStamp] = GETDATE()
    WHERE [ID] IN( SELECT [ObjectID] FROM [deleted]
                   UNION
                   SELECT [ObjectID] FROM [inserted] )
GO

CREATE TRIGGER [images_properties] ON [dbo].[_images]
       FOR INSERT, UPDATE
AS
    -- check for unique property names
    IF( EXISTS( SELECT *
                FROM   [inserted] [i]
                       JOIN
                       [dbo].[_properties] [p] ON
                           [i].[ObjectID] = [p].[ObjectID] AND
                           [i].[Name] = [p].[Name] ) )
    BEGIN
        RAISERROR( 'There is property unique naming conflict!', 11, 1 )
        ROLLBACK TRANSACTION
    END
GO
